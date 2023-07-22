#include <numeric>
#include "matching2D.hpp"

using namespace std;

// Find best matches for keypoints in two camera images based on several matching methods
//MAT_BF, MAT_FLANN, SEL_NN, SEL_KNN
void matchDescriptors(std::vector<cv::KeyPoint> &kPtsSource, std::vector<cv::KeyPoint> &kPtsRef, cv::Mat &descSource, cv::Mat &descRef,
                      std::vector<cv::DMatch> &matches, std::string descriptorType, std::string matcherType, std::string selectorType)
{
    // configure matcher
    bool crossCheck = false;
    cv::Ptr<cv::DescriptorMatcher> matcher;

    if (matcherType.compare("MAT_BF") == 0)
    {
        int normType = cv::NORM_HAMMING;
        matcher = cv::BFMatcher::create(normType, crossCheck);
    }
    else if (matcherType.compare("MAT_FLANN") == 0)
    {
        if (descSource.type() != CV_32F)
        {
            descSource.convertTo(descSource, CV_32F);
            descRef.convertTo(descRef, CV_32F);
        }
        matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
        cout << "FLANN matching";
    }

    // perform matching task
    if (selectorType.compare("SEL_NN") == 0)
    {
        double t = (double)cv::getTickCount();
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        matcher->match(descSource, descRef, matches); // Finds the best match for each descriptor in desc1
        cout << " (SEL_NN) with n=" << matches.size() << " matches in " << 1000 * t / 1.0 << " ms" << endl;
    }
    else if (selectorType.compare("SEL_KNN") == 0)
    {
        vector<vector<cv::DMatch>> k_matches;
        float th{0.8};
        
        double t = (double)cv::getTickCount();
        matcher->knnMatch(descSource, descRef, k_matches, 2);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        
        
        for(auto it{k_matches.begin()}; it!=k_matches.end(); ++it){
            if (it->at(0).distance < it->at(1).distance * th)
                matches.push_back(it->at(0));
        }
        cout << " (SEL_KNN) with n=" << matches.size() << " matches in " << 1000 * t / 1.0 << " ms" << endl;
        cout << "removed " <<  (k_matches.size() - matches.size())/ k_matches.size() * 100 << endl;
    }
}

// Use one of several types of state-of-art descriptors to uniquely identify keypoints
void descKeypoints(vector<cv::KeyPoint> &keypoints, cv::Mat &img, cv::Mat &descriptors, string descriptorType)
{
    // select appropriate descriptor
    cv::Ptr<cv::DescriptorExtractor> extractor;
    if (descriptorType.compare("BRISK") == 0)
    {

        int threshold = 30;        // FAST/AGAST detection threshold score.
        int octaves = 3;           // detection octaves (use 0 to do single scale)
        float patternScale = 1.0f; // apply this scale to the pattern used for sampling the neighbourhood of a keypoint.

        extractor = cv::BRISK::create(threshold, octaves, patternScale);
    }
    else if(descriptorType.compare("AKAZE") == 0)
    {
       extractor = cv::AKAZE::create();
    }
    else if (descriptorType.compare("KAZE") == 0){
        extractor = cv::KAZE::create();
    }else if (descriptorType.compare("MSER") == 0){
        extractor = cv::MSER::create();
    }else if (descriptorType.compare("ORB") == 0){
        extractor = cv::ORB::create();
    }else if (descriptorType.compare("SIFT") == 0){
        extractor = cv::SIFT::create();
    }else
        extractor = cv::xfeatures2d::FREAK::create();

    // perform feature description
    double t = (double)cv::getTickCount();
    extractor->compute(img, keypoints, descriptors);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << descriptorType << " descriptor extraction in " << 1000 * t / 1.0 << " ms" << endl;
}

// Detect keypoints in image using the traditional Shi-Thomasi detector
void detKeypointsShiTomasi(vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    // compute detector parameters based on image size
    int blockSize = 4;       //  size of an average block for computing a derivative covariation matrix over each pixel neighborhood
    double maxOverlap = 0.0; // max. permissible overlap between two features in %
    double minDistance = (1.0 - maxOverlap) * blockSize;
    int maxCorners = img.rows * img.cols / max(1.0, minDistance); // max. num. of keypoints

    double qualityLevel = 0.01; // minimal accepted quality of image corners
    double k = 0.04;

    // Apply corner detection
    double t = (double)cv::getTickCount();
    vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(img, corners, maxCorners, qualityLevel, minDistance, cv::Mat(), blockSize, false, k);

    // add corners to result vector
    for (auto it = corners.begin(); it != corners.end(); ++it)
    {

        cv::KeyPoint newKeyPoint;
        newKeyPoint.pt = cv::Point2f((*it).x, (*it).y);
        newKeyPoint.size = blockSize;
        keypoints.push_back(newKeyPoint);
    }
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "Shi-Tomasi detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "Shi-Tomasi Corner Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        cv::waitKey(0);
    }
}

void detKeypointsHarris(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis){
    
    
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY); // convert to grayscale
    
    int blockSize = 2;
    int apertureSize = 3;
    int minResponse = 100;
    double k = 0.04;

    // Detect Harris corners and normalize output
    double t = (double)cv::getTickCount();
    cv::Mat dst, dst_norm, dst_norm_scaled;
    dst = cv::Mat::zeros(img.size(), CV_32FC1);
    cv::cornerHarris(img, dst, blockSize, apertureSize, k, cv::BORDER_DEFAULT);
    cv::normalize(dst, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
    cv::convertScaleAbs(dst_norm, dst_norm_scaled);

    double crosses{};
    
    for (int i{}; i < dst_norm.rows; i++){
        for (int j{}; j < dst_norm.cols; j++){
            
            int response = (int)dst_norm.at<float>(i, j);
            if (response > minResponse){
                cv::KeyPoint newk;
                newk.pt = cv::Point2f(j, i);
                newk.size = 2 * apertureSize;
                newk.response = response;
                
                bool cross{false};
                
                for (auto it = keypoints.begin(); it != keypoints.end(); ++it){
                    double overlap = cv::KeyPoint::overlap(newk, *it);
                    
                    if (overlap > crosses){
                        cross = true;
                        if (newk.response > it->response){
                            *it = newk;
                            break;
                        }
                    }
                }
                
                if(!cross){
                    keypoints.push_back(newk);
                }
                
            }
            
            
        }
    }
        
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "Harris Corner detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
    
    if(bVis){
        string windowName = "Harris Corner Detection Results";
        cv::namedWindow(windowName, 5);
        cv::Mat visImage = dst_norm_scaled.clone();
        cv::drawKeypoints(dst_norm_scaled, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        cv::imshow(windowName, visImage);
        cv::waitKey(0);
        
    }
}
//FAST, BRISK, ORB, AKAZE, SIFT
void detKeypointsModern(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, std::string detectorType, bool bVis){
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    double t{};
    
    if (detectorType.compare("BRISK") == 0){
        t = (double)cv::getTickCount();
        cv::Ptr<cv::FeatureDetector> detector = cv::BRISK::create();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    }
    else if (detectorType.compare("SIFT") == 0){
        t = (double)cv::getTickCount();
        cv::Ptr<cv::SiftFeatureDetector> detector = cv::SIFT::create();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    }
    
    else if (detectorType.compare("FAST") == 0){
        t = (double)cv::getTickCount();
        cv::Ptr<cv::FastFeatureDetector> detector = cv::FastFeatureDetector::create();
        
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    
    }
    else if (detectorType.compare("ORB") == 0){
        t = (double)cv::getTickCount();
        cv::Ptr<cv::ORB> detector = cv::ORB::create();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    }
    else if(detectorType.compare("AKAZE") == 0){
        t = (double)cv::getTickCount();
        cv::Ptr<cv::AKAZE> detector = cv::AKAZE::create();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    }
    else if(detectorType.compare("HARRIS") == 0){
        detKeypointsHarris(keypoints, img, bVis);
        return;
    }
    else{
        detKeypointsShiTomasi(keypoints, img, bVis);
        return;
    }
    
    
    
    cout << detectorType + " detector with n= " << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
    
    if(bVis){
        string windowName = detectorType + " Detection Results";
        cv::namedWindow(windowName, 5);
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        cv::imshow(windowName, visImage);
        cv::waitKey(0);
        
    }
}
