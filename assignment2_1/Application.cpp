////////////////////////////////////////////////////////////////////////////////
//
// Basic class organizing the application
//
// Authors: Stephan Richter (2011) and Patrick Lühne (2012)
//
////////////////////////////////////////////////////////////////////////////////

#include "Application.h"

#include "DepthCamera.h"
#include "DepthCameraException.h"

////////////////////////////////////////////////////////////////////////////////
//
// Application
//
////////////////////////////////////////////////////////////////////////////////


int frame = 0;
cv::Mat subImage;
cv::Mat thresImage1;
cv::Mat thresImage2;
cv::RotatedRect temp;

void Application::processFrame()
{
	///////////////////////////////////////////////////////////////////////////
	//
	// To do:
	//
	// This method will be called every frame of the camera. Insert code here in
	// order to fulfill the assignment. These images will help you doing so:
	//
	// * m_rgbImage: The image of the Kinect's RGB camera
	// * m_depthImage: The image of the Kinects's depth sensor
	// * m_outputImage: The image in which you can draw the touch circles.
	//
	///////////////////////////////////////////////////////////////////////////
	
	cv::Mat testImage;
	std::vector<std::vector<cv::Point>> contours;

	m_depthImage *= 15;
	m_depthImage.convertTo(testImage, CV_8U, 0.00390625);

	//create background subtraction image and initialize threshold images
	if(frame == 10 || subImage.empty()) {
		frame++;
		m_depthImage.copyTo(subImage);
		subImage.convertTo(subImage, CV_8U, 0.00390625);	
		subImage = cv::Scalar::all(255) - subImage;
		m_depthImage.convertTo(thresImage1, CV_8U, 0.00390625);
		m_depthImage.convertTo(thresImage2, CV_8U, 0.00390625);
	}
	//blur modified depthImage, bitwise_and testImage with both threshold images, find contours
	else {
		frame++;
		testImage = (subImage + testImage)/2;
		cv::GaussianBlur(testImage, testImage, cv::Size(5,5),0,0);
		
		cv::threshold(testImage, thresImage1, 125, 255, 0);
		cv::threshold(testImage, thresImage2, 126, 255, 1);

		cv::bitwise_and(thresImage1, thresImage2, testImage);
		cv::GaussianBlur(testImage, testImage, cv::Size(5,5),0,0);
		cv::Mat element = cv::getStructuringElement(  cv::MORPH_RECT, cv::Size(15,15), cv::Point( -1, -1 ) );
		cv::erode(testImage, testImage, element);
		//testImage.copyTo(m_outputImage);

		cv::findContours(testImage, contours, 0, 1, cv::Point(0,0));
	}

	//initialize variables needed for ellipse
	cv::vector<cv::RotatedRect> minEllipse(contours.size());
	cv::Scalar color = cv::Scalar(255,255,255);
	cv::RotatedRect biggest;
	biggest.size = cv::Size2f(10,10);

	//find biggest ellipse
	for(int i = 0; i < contours.size(); i++) {
		if(contours[i].size() > 5) {
			minEllipse[i] = cv::fitEllipse(contours[i]);
			if(minEllipse[i].size.area() > biggest.size.area());
				biggest = minEllipse[i];
		}
	}
	cv::ellipse(m_rgbImage, biggest, color, 2, 8);
	cv::circle(m_outputImage, biggest.center, 20, cv::Scalar::all(255), CV_FILLED);
}

////////////////////////////////////////////////////////////////////////////////

Application::Application()
{
	m_isFinished = false;

	try
	{
		m_depthCamera = new DepthCamera;
	}
	catch (DepthCameraException)
	{
		m_isFinished = true;
		return;
	}

    // open windows
	cv::namedWindow("output", 1);
	cv::namedWindow("depth", 1);
	cv::namedWindow("raw", 1);

    // create work buffer
	m_rgbImage = cv::Mat(480, 640, CV_8UC3);
	m_depthImage = cv::Mat(480, 640, CV_16UC1),
	m_outputImage = cv::Mat(480, 640, CV_8UC1);
}

////////////////////////////////////////////////////////////////////////////////

Application::~Application()
{
	if (m_depthCamera)
		delete m_depthCamera;
}

////////////////////////////////////////////////////////////////////////////////

void Application::loop()
{
	// Check for key input
	int key = cv::waitKey(20);

	switch (key)
	{
		case 's':
			makeScreenshots();
			break;

		case 'c':
			clearOutputImage();
			break;

		case 'q':
			m_isFinished = true;
	}

	// Grab new images from the Kinect's cameras
	m_depthCamera->frameFromCamera(m_rgbImage, m_depthImage, CV_16UC1);

	// Process the current frame
	processFrame();

	// Display the images
	cv::imshow("raw", m_rgbImage);
	cv::imshow("depth", m_depthImage);
	cv::imshow("output", m_outputImage);
}

////////////////////////////////////////////////////////////////////////////////

void Application::makeScreenshots()
{
	cv::imwrite("raw.png", m_rgbImage);
	cv::imwrite("depth.png", m_depthImage);
	cv::imwrite("output.png", m_outputImage);
}

////////////////////////////////////////////////////////////////////////////////

void Application::clearOutputImage()
{
	cv::rectangle(m_outputImage, cv::Point(0, 0), cv::Point(640, 480),
				  cv::Scalar::all(0), CV_FILLED);
}

////////////////////////////////////////////////////////////////////////////////

bool Application::isFinished()
{
	return m_isFinished;
}
