//Jose Joseph Thandapral
//CS5330
//Project 2 : Content-based Image Retrieval
// Project involving
//

#include<opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <csv_util.h>
using namespace cv;
using namespace std;

//prototypes of functions used to create sobel X, sobel Y and gradient magnitude image 
int sobelX3x3(cv::Mat& src, cv::Mat& dst);
int sobelY3x3(cv::Mat& src, cv::Mat& dst);
int magnitude(cv::Mat& sx, cv::Mat& sy, cv::Mat& dst);

char dirname[256] = "C:/Users/josei/OneDrive/Documents/NEU/Assignments/Assignments_PRCV CS5330/project 2/olympus";
char buffer[256] = "";
FILE* fp;
DIR* dirp = opendir(dirname);
struct dirent* dp;

//function that normalizes the 16 bin histogram values to a double value between 0 and 1
void normalize(double* arr, int size = 16) {
    double minVal = arr[0];
    double maxVal = arr[0];

    // Find the maximum and minimum values in the array
    for (int i = 1; i < size; i++) {
        if (arr[i] < minVal) {
            minVal = arr[i];
        }
        if (arr[i] > maxVal) {
            maxVal = arr[i];
        }
    }

    // Shift the range of values so that the minimum value is now 0
    double range = maxVal - minVal;
    for (int i = 0; i < size; i++) {
        arr[i] -= minVal;
    }

    // Scale the values to a range of 0 to 1
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] / range;
    }
}

//Task 1: Baseline matching with the central 9x9 pixels of target image and other images
int task1()
{
    cv::Mat targetImage;
    vector<pair<double, string>> distances; //vector pair object of the summed distance and image file path
    cv::Mat frame;

    //Taking pic.1016.jpg as target image
    targetImage = cv::imread("C:/Users/josei/OneDrive/Documents/NEU/Assignments/Assignments_PRCV CS5330/project 2/olympus/pic.1016.jpg");
    cv::namedWindow("ref", 1);
    cv::imshow("ref", targetImage);
    cv::namedWindow("Frame1", 1);
    cv::namedWindow("Frame2", 1);
    cv::namedWindow("Frame3", 1);

    //decalring a Rect object with central 9x9 square matrix
    Rect roi((targetImage.cols - 9) / 2, (targetImage.rows - 9) / 2, 9, 9);
    //Getting Mat object with central 9x9 square matrix
    Mat target = targetImage(roi);

    // get the directory path
    printf("Processing directory %s\n", dirname);
    if (dirp == NULL) {
        printf("Cannot open directory %s\n", dirname);
        exit(-1);
    }
    // loop over all the files in the image file listing
    while ((dp = readdir(dirp)) != NULL) {
        // check if the file is an image
        if (strstr(dp->d_name, ".jpg") ||
            strstr(dp->d_name, ".png") ||
            strstr(dp->d_name, ".ppm") ||
            strstr(dp->d_name, ".tif"))
        {
            printf("processing image file: %s\n", dp->d_name);
            // build the overall filename
            strcpy_s(buffer, dirname);
            strcat_s(buffer, "/");
            strcat_s(buffer, dp->d_name);
            printf("full path name: %s", buffer);

            frame = cv::imread(buffer); //Read the current image from directory
            Rect roi((frame.cols - 9) / 2, (frame.rows - 9) / 2, 9, 9); //get central 9x9 matrix of current image
            Mat target1 = frame(roi);
            Mat diff;
            absdiff(target1, target, diff); //difference between target and current image mat frames
            diff = diff.mul(diff); //L2 norm distance of frames
            Scalar sum = cv::sum(diff);  //storing sum of R,G and B color space L2 norm distances in Scalar type object
            double dist = sum[0] + sum[1] + sum[2]; //Taking sum of L2 norm distances of color spaces 

            distances.emplace_back(dist, buffer);  //Append the summed distance and image file path to vector pair object
        }
    }
    int p = 0;
    sort(distances.begin(), distances.end()); //Sort the vector pairs according to distances
    Mat dispImg = imread(distances[1].second);
    //Display the first 3 matches and their file paths in output terminal
    cv::imshow("Frame1", dispImg);
    printf("\nMatch #1:");
    p = 0;
    while (distances[1].second[p] != '\0')
    {
        printf("%c", distances[1].second[p]);
        p++;
    }
    dispImg = imread(distances[2].second);
    cv::imshow("Frame2", dispImg);
    printf("\nMatch #2:");
    p = 0;
    while (distances[2].second[p] != '\0')
    {
        printf("%c", distances[2].second[p]);
        p++;
    }
    dispImg = imread(distances[3].second);
    cv::imshow("Frame3", dispImg);
    printf("\nMatch #3:");
    p = 0;
    while (distances[3].second[p] != '\0')
    {
        printf("%c", distances[3].second[p]);
        p++;
    }
    while (cv::waitKey(1) != 'q');
    cv::destroyAllWindows();
    closedir(dirp);
    return 0;
}

//Task 2: Histogram matching of target image and other images
int task2()
{
    cv::Mat targetImage;
    vector<pair<double, string>> distances;//vector pair object of the summed distance and image file path
    cv::Mat frame;
    int idx;

    //Taking pic.0164.jpg as target image
    targetImage = cv::imread("C:/Users/josei/OneDrive/Documents/NEU/Assignments/Assignments_PRCV CS5330/project 2/olympus/pic.0164.jpg");
    cv::namedWindow("ref", 1);
    cv::imshow("ref", targetImage);
    cv::namedWindow("Frame1", 1);
    cv::namedWindow("Frame2", 1);
    cv::namedWindow("Frame3", 1);
    double r_bin[16];
    double g_bin[16];
    //assigning the 16 bins to be 0 initially
    for (int i = 0; i < 16; ++i)
    {
        r_bin[i] = 0;
        g_bin[i] = 0;

    }
    //calculating histogram of target image
    for (int i = 0; i < targetImage.rows; ++i)
    {
        cv::Vec3b* rptrs = targetImage.ptr <cv::Vec3b>(i);
        for (int j = 0; j < targetImage.cols; ++j)
        {
            //Adding 1 to each bin value based on occurence of color intensity in R,G and B color spaces
            idx = rptrs[j][2];
            int k = ((idx - (idx % 16)) / 16);
            r_bin[k] = r_bin[k] + 1;
            idx = rptrs[j][0];
            k = ((idx - (idx % 16)) / 16);
            g_bin[k] = g_bin[k] + 1;
        }
    }

    //Normalize the 16 bin color spaces to between 0 and 1 double type values
    normalize(r_bin);
    normalize(g_bin);
    // get the directory path
    printf("Processing directory %s\n", dirname);
    if (dirp == NULL) {
        printf("Cannot open directory %s\n", dirname);
        exit(-1);
    }
    // loop over all the files in the image file listing
    while ((dp = readdir(dirp)) != NULL) {
        // check if the file is an image
        if (strstr(dp->d_name, ".jpg") ||
            strstr(dp->d_name, ".png") ||
            strstr(dp->d_name, ".ppm") ||
            strstr(dp->d_name, ".tif"))
        {
            printf("processing image file: %s\n", dp->d_name);
            // build the overall filename
            strcpy_s(buffer, dirname);
            strcat_s(buffer, "/");
            strcat_s(buffer, dp->d_name);
            printf("full path name: %s", buffer);

            //assigning the 16 bins to be 0 initially
            double r_bin1[16];
            double g_bin1[16];

            frame = cv::imread(buffer);

            for (int i = 0; i < 16; ++i)
            {
                r_bin1[i] = 0;
                g_bin1[i] = 0;

            }

            //calculating histogram of image
            for (int i = 0; i < frame.rows; ++i)
            {
                cv::Vec3b* rptrd = frame.ptr <cv::Vec3b>(i);
                for (int j = 0; j < frame.cols; ++j)
                {
                    //Adding 1 to each bin value based on occurence of color intensity in R,G and B color spaces
                    idx = rptrd[j][2];
                    int k = ((idx - (idx % 16)) / 16);
                    r_bin1[k] = r_bin1[k] + 1;
                    idx = rptrd[j][0];
                    k = ((idx - (idx % 16)) / 16);
                    g_bin1[k] = g_bin1[k] + 1;
                }
            }

            //Normalize the 16 bin color spaces to between 0 and 1 double type values
            normalize(r_bin1);
            normalize(g_bin1);

            //Taking sum of L2 norm distances of color space 16 bin histograms
            double rdif = 0, gdif = 0;
            for (int i = 0; i < 16; ++i)
            {
                rdif = rdif + (abs(r_bin1[i] - r_bin[i]) * abs(r_bin1[i] - r_bin[i]));
                gdif = gdif + (abs(g_bin1[i] - g_bin[i]) * abs(g_bin1[i] - g_bin[i]));
            }
            rdif = sqrt(rdif);
            gdif = sqrt(gdif);
            double dist = rdif + gdif;
            distances.emplace_back(dist, buffer); //Append the summed distance and image file path to vector pair object
        }
    }

    int p = 0;
    sort(distances.begin(), distances.end());//Sort the vector pairs according to distances
    Mat dispImg = imread(distances[1].second);
    //Display the first 3 matches and their file paths in output terminal
    cv::imshow("Frame1", dispImg);
    printf("\nMatch #1:");
    p = 0;
    while (distances[1].second[p] != '\0')
    {
        printf("%c", distances[1].second[p]);
        p++;
    }
    dispImg = imread(distances[2].second);
    cv::imshow("Frame2", dispImg);
    printf("\nMatch #2:");
    p = 0;
    while (distances[2].second[p] != '\0')
    {
        printf("%c", distances[2].second[p]);
        p++;
    }
    dispImg = imread(distances[3].second);
    cv::imshow("Frame3", dispImg);
    printf("\nMatch #3:");
    p = 0;
    while (distances[3].second[p] != '\0')
    {
        printf("%c", distances[3].second[p]);
        p++;
    }
    while (cv::waitKey(1) != 'q');
    cv::destroyAllWindows();
    closedir(dirp);
    return 0;
}

int task3()
{
    cv::Mat targetImage;
    vector<pair<double, string>> distances;//vector pair object of the summed distance and image file path
    cv::Mat frame;
    int idx;

    //Taking pic.0274.jpg as target image
    targetImage = cv::imread("C:/Users/josei/OneDrive/Documents/NEU/Assignments/Assignments_PRCV CS5330/project 2/olympus/pic.0274.jpg");
    Rect roi((targetImage.cols - 9) / 2, (targetImage.rows - 9) / 2, 9, 9);//decalring a Rect object with central 9x9 square matrix
    Mat targetCenter = targetImage(roi);
    cv::namedWindow("ref", 1);
    cv::imshow("ref", targetImage);
    cv::namedWindow("Frame1", 1);
    cv::namedWindow("Frame2", 1);
    cv::namedWindow("Frame3", 1);
    double r_bin[16];
    double g_bin[16];
    //assigning the 16 bins to be 0 initially
    for (int i = 0; i < 16; ++i)
    {
        r_bin[i] = 0;
        g_bin[i] = 0;

    }
    //calculating histogram of target image
    for (int i = 0; i < targetImage.rows; ++i)
    {
        //Adding 1 to each bin value based on occurence of color intensity in R,G and B color spaces
        cv::Vec3b* rptrs = targetImage.ptr <cv::Vec3b>(i);
        for (int j = 0; j < targetImage.cols; ++j)
        {
            idx = rptrs[j][2];
            int k = ((idx - (idx % 16)) / 16);
            r_bin[k] = r_bin[k] + 1;
            idx = rptrs[j][0];
            k = ((idx - (idx % 16)) / 16);
            g_bin[k] = g_bin[k] + 1;
        }
    }
    //Normalize the 16 bin color spaces to between 0 and 1 double type values
    normalize(r_bin);
    normalize(g_bin);

    //calculate histogram of center of target image
    double r_bink[16];
    double g_bink[16];
    //assigning the 16 bins to be 0 initially
    for (int i = 0; i < 16; ++i)
    {
        r_bink[i] = 0;
        g_bink[i] = 0;

    }

    for (int i = 0; i < targetCenter.rows; ++i)
    {
        //Adding 1 to each bin value based on occurence of color intensity in R,G and B color spaces
        cv::Vec3b* rptrk = targetCenter.ptr <cv::Vec3b>(i);
        for (int j = 0; j < targetCenter.cols; ++j)
        {
            idx = rptrk[j][2];
            int k = ((idx - (idx % 16)) / 16);
            r_bink[k] = r_bink[k] + 1;
            idx = rptrk[j][0];
            k = ((idx - (idx % 16)) / 16);
            g_bink[k] = g_bink[k] + 1;
        }
    }
    //Normalize the 16 bin color spaces to between 0 and 1 double type values
    normalize(r_bink);
    normalize(g_bink);
    // get the directory path
    printf("Processing directory %s\n", dirname);
    if (dirp == NULL) {
        printf("Cannot open directory %s\n", dirname);
        exit(-1);
    }
    // loop over all the files in the image file listing
    while ((dp = readdir(dirp)) != NULL) {
        // check if the file is an image
        if (strstr(dp->d_name, ".jpg") ||
            strstr(dp->d_name, ".png") ||
            strstr(dp->d_name, ".ppm") ||
            strstr(dp->d_name, ".tif"))
        {
            printf("processing image file: %s\n", dp->d_name);
            // build the overall filename
            strcpy_s(buffer, dirname);
            strcat_s(buffer, "/");
            strcat_s(buffer, dp->d_name);
            printf("full path name: %s", buffer);


            double r_bin1[16];
            double g_bin1[16];

            frame = cv::imread(buffer);
            //assigning the 16 bins to be 0 initially
            for (int i = 0; i < 16; ++i)
            {
                r_bin1[i] = 0;
                g_bin1[i] = 0;

            }

            //calculating histogram of image
            for (int i = 0; i < frame.rows; ++i)
            {
                //Adding 1 to each bin value based on occurence of color intensity in R,G and B color spaces
                cv::Vec3b* rptrd = frame.ptr <cv::Vec3b>(i);
                for (int j = 0; j < frame.cols; ++j)
                {
                    idx = rptrd[j][2];
                    int k = ((idx - (idx % 16)) / 16);
                    r_bin1[k] = r_bin1[k] + 1;
                    idx = rptrd[j][0];
                    k = ((idx - (idx % 16)) / 16);
                    g_bin1[k] = g_bin1[k] + 1;
                }
            }
            //Normalize the 16 bin color spaces to between 0 and 1 double type values
            normalize(r_bin1);
            normalize(g_bin1);

            //Taking sum of L2 norm distances of color space 16 bin histograms
            double rdif = 0, gdif = 0;
            for (int i = 0; i < 16; ++i)
            {
                rdif = rdif + (abs(r_bin1[i] - r_bin[i]) * abs(r_bin1[i] - r_bin[i]));
                gdif = gdif + (abs(g_bin1[i] - g_bin[i]) * abs(g_bin1[i] - g_bin[i]));
            }
            rdif = sqrt(rdif);
            gdif = sqrt(gdif);
            double dist = rdif + gdif;


            //calculate histogram of central image
            frame = cv::imread(buffer);
            int a = (frame.cols - 9) / 2;
            Rect roi((frame.cols - 9) / 2, (frame.rows - 9) / 2, 9, 9);
            Mat targetCenter1 = frame(roi);

            double r_bin2[16];
            double g_bin2[16];
            //assigning the 16 bins to be 0 initially
            for (int i = 0; i < 16; ++i)
            {
                r_bin2[i] = 0;
                g_bin2[i] = 0;

            }

            for (int i = 0; i < targetCenter1.rows; ++i)
            {
                //Adding 1 to each bin value based on occurence of color intensity in R,G and B color spaces
                cv::Vec3b* rptrt = targetCenter1.ptr <cv::Vec3b>(i);
                for (int j = 0; j < targetCenter1.cols; ++j)
                {
                    idx = rptrt[j][2];
                    int k = ((idx - (idx % 16)) / 16);
                    r_bin2[k] = r_bin2[k] + 1;
                    idx = rptrt[j][0];
                    k = ((idx - (idx % 16)) / 16);
                    g_bin2[k] = g_bin2[k] + 1;
                }
            }
            //Normalize the 16 bin color spaces to between 0 and 1 double type values
            normalize(r_bin2);
            normalize(g_bin2);

            //Taking sum of L2 norm distances of color space 16 bin histograms
            double rdif1 = 0, gdif1 = 0;
            for (int i = 0; i < 16; ++i)
            {
                rdif1 = rdif1 + (abs(r_bin2[i] - r_bink[i]) * abs(r_bin2[i] - r_bink[i]));
                gdif1 = gdif1 + (abs(g_bin2[i] - g_bink[i]) * abs(g_bin2[i] - g_bink[i]));
            }
            rdif1 = sqrt(rdif1);
            gdif1 = sqrt(gdif1);
            double dist1 = rdif1 + gdif1;

            dist = dist + dist1;

            distances.emplace_back(dist, buffer);//Append the summed distance and image file path to vector pair object
        }
    }

    int p = 0;
    sort(distances.begin(), distances.end());//Sort the vector pairs according to distances
    Mat dispImg = imread(distances[1].second);
    cv::imshow("Frame1", dispImg);
    printf("\nMatch #1:");
    p = 0;
    while (distances[1].second[p] != '\0')
    {
        printf("%c", distances[1].second[p]);
        p++;
    }
    dispImg = imread(distances[2].second);
    cv::imshow("Frame2", dispImg);
    printf("\nMatch #2:");
    p = 0;
    while (distances[2].second[p] != '\0')
    {
        printf("%c", distances[2].second[p]);
        p++;
    }
    dispImg = imread(distances[3].second);
    cv::imshow("Frame3", dispImg);
    printf("\nMatch #3:");
    p = 0;
    while (distances[3].second[p] != '\0')
    {
        printf("%c", distances[3].second[p]);
        p++;
    }
    while (cv::waitKey(1) != 'q');
    cv::destroyAllWindows();
    closedir(dirp);
    return 0;
}

int task4()
{
    cv::Mat targetImage;
    cv::Mat sobelXImaget;
    cv::Mat sobelYImaget;
    cv::Mat magImaget;
    cv::Mat sobelXImage;
    cv::Mat sobelYImage;
    cv::Mat magImage;
    cv::Mat sobelXdisplaysrct;
    cv::Mat sobelYdisplaysrct;
    cv::Mat sobelXdisplaysrc;
    cv::Mat sobelYdisplaysrc;
    vector<pair<double, string>> distances;
    cv::Mat frame;
    int idx;

    //Taking pic.0535.jpg as target image
    targetImage = cv::imread("C:/Users/josei/OneDrive/Documents/NEU/Assignments/Assignments_PRCV CS5330/project 2/olympus/pic.0535.jpg");
    sobelX3x3(targetImage, sobelXImaget);
    cv::convertScaleAbs(sobelXImaget, sobelXdisplaysrct);
    sobelY3x3(targetImage, sobelYImaget);
    cv::convertScaleAbs(sobelYImaget, sobelYdisplaysrct);
    magImaget.create(targetImage.size(), targetImage.type());
    magnitude(sobelXdisplaysrct, sobelYdisplaysrct, magImaget);//Getting the gradient magnitude image
    cv::namedWindow("ref", 1);
    cv::imshow("ref", targetImage);
    cv::namedWindow("Frame1", 1);
    cv::namedWindow("Frame2", 1);
    cv::namedWindow("Frame3", 1);
    double r_bin[16];
    double g_bin[16];
    //assigning the 16 bins to be 0 initially
    for (int i = 0; i < 16; ++i)
    {
        r_bin[i] = 0;
        g_bin[i] = 0;

    }
    //calculating histogram of target image
    for (int i = 0; i < targetImage.rows; ++i)
    {
        cv::Vec3b* rptrs = targetImage.ptr <cv::Vec3b>(i);
        for (int j = 0; j < targetImage.cols; ++j)
        {
            idx = rptrs[j][2];
            int k = ((idx - (idx % 16)) / 16);
            r_bin[k] = r_bin[k] + 1;
            idx = rptrs[j][0];
            k = ((idx - (idx % 16)) / 16);
            g_bin[k] = g_bin[k] + 1;
        }
    }
    //Normalize the 16 bin color spaces to between 0 and 1 double type values
    normalize(r_bin);
    normalize(g_bin);

    //calculate histogram of sobel magnitude image of target image
    double r_bink[16];
    double g_bink[16];
    //assigning the 16 bins to be 0 initially
    for (int i = 0; i < 16; ++i)
    {
        r_bink[i] = 0;
        g_bink[i] = 0;

    }

    for (int i = 0; i < magImaget.rows; ++i)
    {
        cv::Vec3b* rptrk = magImaget.ptr <cv::Vec3b>(i);
        for (int j = 0; j < magImaget.cols; ++j)
        {
            idx = rptrk[j][2];
            int k = ((idx - (idx % 16)) / 16);
            r_bink[k] = r_bink[k] + 1;
            idx = rptrk[j][0];
            k = ((idx - (idx % 16)) / 16);
            g_bink[k] = g_bink[k] + 1;
        }
    }

    normalize(r_bink);
    normalize(g_bink);

    // get the directory path
    printf("Processing directory %s\n", dirname);
    if (dirp == NULL) {
        printf("Cannot open directory %s\n", dirname);
        exit(-1);
    }
    // loop over all the files in the image file listing
    while ((dp = readdir(dirp)) != NULL) {
        // check if the file is an image
        if (strstr(dp->d_name, ".jpg") ||
            strstr(dp->d_name, ".png") ||
            strstr(dp->d_name, ".ppm") ||
            strstr(dp->d_name, ".tif"))
        {
            printf("processing image file: %s\n", dp->d_name);
            // build the overall filename
            strcpy_s(buffer, dirname);
            strcat_s(buffer, "/");
            strcat_s(buffer, dp->d_name);
            printf("full path name: %s", buffer);


            double r_bin1[16];
            double g_bin1[16];

            frame = cv::imread(buffer);

            for (int i = 0; i < 16; ++i)
            {
                r_bin1[i] = 0;
                g_bin1[i] = 0;

            }

            //calculating histogram of image
            for (int i = 0; i < frame.rows; ++i)
            {
                cv::Vec3b* rptrd = frame.ptr <cv::Vec3b>(i);
                for (int j = 0; j < frame.cols; ++j)
                {
                    idx = rptrd[j][2];
                    int k = ((idx - (idx % 16)) / 16);
                    r_bin1[k] = r_bin1[k] + 1;
                    idx = rptrd[j][0];
                    k = ((idx - (idx % 16)) / 16);
                    g_bin1[k] = g_bin1[k] + 1;
                }
            }


            normalize(r_bin1);
            normalize(g_bin1);

            double rdif = 0, gdif = 0;
            for (int i = 0; i < 16; ++i)
            {
                rdif = rdif + (abs(r_bin1[i] - r_bin[i]) * abs(r_bin1[i] - r_bin[i]));
                gdif = gdif + (abs(g_bin1[i] - g_bin[i]) * abs(g_bin1[i] - g_bin[i]));
            }
            rdif = sqrt(rdif);
            gdif = sqrt(gdif);
            double dist = rdif + gdif;


            //calculate histogram of sobel magnitude image of central image
            frame = cv::imread(buffer);
            sobelX3x3(frame, sobelXImage);
            cv::convertScaleAbs(sobelXImage, sobelXdisplaysrc);
            sobelY3x3(frame, sobelYImage);
            cv::convertScaleAbs(sobelYImage, sobelYdisplaysrc);
            magImage.create(frame.size(), frame.type());
            magnitude(sobelXdisplaysrc, sobelYdisplaysrc, magImage);
            Mat targetCenter1 = magImage;

            double r_bin2[16];
            double g_bin2[16];
            for (int i = 0; i < 16; ++i)
            {
                r_bin2[i] = 0;
                g_bin2[i] = 0;

            }

            for (int i = 0; i < targetCenter1.rows; ++i)
            {
                cv::Vec3b* rptrt = targetCenter1.ptr <cv::Vec3b>(i);
                for (int j = 0; j < targetCenter1.cols; ++j)
                {
                    idx = rptrt[j][2];
                    int k = ((idx - (idx % 16)) / 16);
                    r_bin2[k] = r_bin2[k] + 1;
                    idx = rptrt[j][0];
                    k = ((idx - (idx % 16)) / 16);
                    g_bin2[k] = g_bin2[k] + 1;
                }
            }

            normalize(r_bin2);
            normalize(g_bin2);

            double rdif1 = 0, gdif1 = 0;
            for (int i = 0; i < 16; ++i)
            {
                rdif1 = rdif1 + (abs(r_bin2[i] - r_bink[i]) * abs(r_bin2[i] - r_bink[i]));
                gdif1 = gdif1 + (abs(g_bin2[i] - g_bink[i]) * abs(g_bin2[i] - g_bink[i]));
            }
            rdif1 = sqrt(rdif1);
            gdif1 = sqrt(gdif1);
            double dist1 = rdif1 + gdif1;

            dist = dist + dist1;

            distances.emplace_back(dist, buffer);
        }
    }

    int p = 0;
    sort(distances.begin(), distances.end());
    Mat dispImg = imread(distances[1].second);
    cv::imshow("Frame1", dispImg);
    printf("\nMatch #1:");
    p = 0;
    while (distances[1].second[p] != '\0')
    {
        printf("%c", distances[1].second[p]);
        p++;
    }
    dispImg = imread(distances[2].second);
    cv::imshow("Frame2", dispImg);
    printf("\nMatch #2:");
    p = 0;
    while (distances[2].second[p] != '\0')
    {
        printf("%c", distances[2].second[p]);
        p++;
    }
    dispImg = imread(distances[3].second);
    cv::imshow("Frame3", dispImg);
    printf("\nMatch #3:");
    p = 0;
    while (distances[3].second[p] != '\0')
    {
        printf("%c", distances[3].second[p]);
        p++;
    }
    while (cv::waitKey(1) != 'q');
    cv::destroyAllWindows();
    closedir(dirp);
    return 0;
}

int task5()
{
    cv::Mat targetImage;
    cv::Mat targetImage1[2];
    cv::Mat sobelXImaget;
    cv::Mat sobelYImaget;
    cv::Mat magImaget;
    cv::Mat sobelXImage;
    cv::Mat sobelYImage;
    cv::Mat magImage;
    cv::Mat sobelXdisplaysrct;
    cv::Mat sobelYdisplaysrct;
    cv::Mat sobelXdisplaysrc;
    cv::Mat sobelYdisplaysrc;
    double r_bin[16];
    double g_bin[16];
    vector<pair<double, string>> distances;
    cv::Mat frame;
    int idx;
    char dirname1[256] = "C:/Users/josei/OneDrive/Documents/NEU/Assignments/Assignments_PRCV CS5330/project 2/olympus1";
    char buffer[256] = "";
    DIR* dirp1 = opendir(dirname1);
    struct dirent* dp1;

    //Getting the 4 images that have the basketball image in a prominent part of the frame
    targetImage1[0] = imread("C:/Users/josei/OneDrive/Documents/NEU/Assignments/Assignments_PRCV CS5330/project 2/olympus1/pic.0280.jpg");
    targetImage1[1] = imread("C:/Users/josei/OneDrive/Documents/NEU/Assignments/Assignments_PRCV CS5330/project 2/olympus1/pic.0281.jpg");
    for (int i = 0; i < 16; ++i)
    {
        r_bin[i] = 0;
        g_bin[i] = 0;

    }


    double r_bink[16];
    double g_bink[16];
    for (int i = 0; i < 16; ++i)
    {
        r_bink[i] = 0;
        g_bink[i] = 0;

    }

    //Taking pic.0343.jpg as target image
    for (int i = 0; i < 2; ++i)
    {
        targetImage = targetImage1[i];

        sobelX3x3(targetImage, sobelXImaget);
        cv::convertScaleAbs(sobelXImaget, sobelXdisplaysrct);
        sobelY3x3(targetImage, sobelYImaget);
        cv::convertScaleAbs(sobelYImaget, sobelYdisplaysrct);
        magImaget.create(targetImage.size(), targetImage.type());
        magnitude(sobelXdisplaysrct, sobelYdisplaysrct, magImaget);
        cv::namedWindow("ref1", 1);
        cv::imshow("ref1", targetImage1[0]);
        cv::namedWindow("ref2", 1);
        cv::imshow("ref2", targetImage1[1]);
        cv::namedWindow("Frame1", 1);
        cv::namedWindow("Frame2", 1);
        cv::namedWindow("Frame3", 1);
        cv::namedWindow("Frame4", 1);
        cv::namedWindow("Frame5", 1);
        cv::namedWindow("Frame6", 1);
        cv::namedWindow("Frame7", 1);
        cv::namedWindow("Frame8", 1);
        cv::namedWindow("Frame9", 1);
        cv::namedWindow("Frame10", 1);
        //calculating histogram of target image
        for (int i = 0; i < targetImage.rows; ++i)
        {
            cv::Vec3b* rptrs = targetImage.ptr <cv::Vec3b>(i);
            for (int j = 0; j < targetImage.cols; ++j)
            {
                idx = rptrs[j][2];
                int k = ((idx - (idx % 16)) / 16);
                r_bin[k] = r_bin[k] + 1;
                idx = rptrs[j][0];
                k = ((idx - (idx % 16)) / 16);
                g_bin[k] = g_bin[k] + 1;
            }
        }


        //calculate histogram of sobel magnitude image of target image

        for (int i = 0; i < magImaget.rows; ++i)
        {
            cv::Vec3b* rptrk = magImaget.ptr <cv::Vec3b>(i);
            for (int j = 0; j < magImaget.cols; ++j)
            {
                idx = rptrk[j][2];
                int k = ((idx - (idx % 16)) / 16);
                r_bink[k] = r_bink[k] + 1;
                idx = rptrk[j][0];
                k = ((idx - (idx % 16)) / 16);
                g_bink[k] = g_bink[k] + 1;
            }
        }
    }
    normalize(r_bink);
    normalize(g_bink);
    normalize(r_bin);
    normalize(g_bin);

    // get the directory path
    printf("Processing directory %s\n", dirname1);
    if (dirp1 == NULL) {
        printf("Cannot open directory %s\n", dirname1);
        exit(-1);
    }
    // loop over all the files in the image file listing
    while ((dp = readdir(dirp1)) != NULL) {
        // check if the file is an image
        if (strstr(dp->d_name, ".jpg") ||
            strstr(dp->d_name, ".png") ||
            strstr(dp->d_name, ".ppm") ||
            strstr(dp->d_name, ".tif"))
        {
            printf("processing image file: %s\n", dp->d_name);
            // build the overall filename
            strcpy_s(buffer, dirname);
            strcat_s(buffer, "/");
            strcat_s(buffer, dp->d_name);
            printf("full path name: %s", buffer);


            double r_bin1[16];
            double g_bin1[16];

            frame = cv::imread(buffer);

            for (int i = 0; i < 16; ++i)
            {
                r_bin1[i] = 0;
                g_bin1[i] = 0;

            }

            //calculating histogram of image
            for (int i = 0; i < frame.rows; ++i)
            {
                cv::Vec3b* rptrd = frame.ptr <cv::Vec3b>(i);
                for (int j = 0; j < frame.cols; ++j)
                {
                    idx = rptrd[j][2];
                    int k = ((idx - (idx % 16)) / 16);
                    r_bin1[k] = r_bin1[k] + 1;
                    idx = rptrd[j][0];
                    k = ((idx - (idx % 16)) / 16);
                    g_bin1[k] = g_bin1[k] + 1;
                }
            }


            normalize(r_bin1);
            normalize(g_bin1);

            double rdif = 0, gdif = 0;
            for (int i = 0; i < 16; ++i)
            {
                rdif = rdif + (abs(r_bin1[i] - r_bin[i]) * abs(r_bin1[i] - r_bin[i]));
                gdif = gdif + (abs(g_bin1[i] - g_bin[i]) * abs(g_bin1[i] - g_bin[i]));
            }
            rdif = sqrt(rdif);
            gdif = sqrt(gdif);
            double dist = rdif + gdif;


            //calculate histogram of sobel magnitude image of central image
            frame = cv::imread(buffer);
            sobelX3x3(frame, sobelXImage);
            cv::convertScaleAbs(sobelXImage, sobelXdisplaysrc);
            sobelY3x3(frame, sobelYImage);
            cv::convertScaleAbs(sobelYImage, sobelYdisplaysrc);
            magImage.create(frame.size(), frame.type());
            magnitude(sobelXdisplaysrc, sobelYdisplaysrc, magImage);
            Mat targetCenter1 = magImage;

            double r_bin2[16];
            double g_bin2[16];
            for (int i = 0; i < 16; ++i)
            {
                r_bin2[i] = 0;
                g_bin2[i] = 0;

            }

            for (int i = 0; i < targetCenter1.rows; ++i)
            {
                cv::Vec3b* rptrt = targetCenter1.ptr <cv::Vec3b>(i);
                for (int j = 0; j < targetCenter1.cols; ++j)
                {
                    idx = rptrt[j][2];
                    int k = ((idx - (idx % 16)) / 16);
                    r_bin2[k] = r_bin2[k] + 1;
                    idx = rptrt[j][0];
                    k = ((idx - (idx % 16)) / 16);
                    g_bin2[k] = g_bin2[k] + 1;
                }
            }

            normalize(r_bin2);
            normalize(g_bin2);

            double rdif1 = 0, gdif1 = 0;
            for (int i = 0; i < 16; ++i)
            {
                rdif1 = rdif1 + (abs(r_bin2[i] - r_bink[i]) * abs(r_bin2[i] - r_bink[i]));
                gdif1 = gdif1 + (abs(g_bin2[i] - g_bink[i]) * abs(g_bin2[i] - g_bink[i]));
            }
            rdif1 = sqrt(rdif1);
            gdif1 = sqrt(gdif1);
            double dist1 = rdif1 + gdif1;

            //Fine-tuned values of a1*dist+a2+dist1 and  to get image matches close to object required in image 
            dist = 0.7*dist + 2*dist1;

            distances.emplace_back(dist, buffer);
        }
    }

    int p = 0;
    sort(distances.begin(), distances.end());
    Mat dispImg = imread(distances[0].second);
    cv::imshow("Frame1", dispImg);
    printf("\nMatch #1:");
    p = 0;
    while (distances[0].second[p] != '\0')
    {
        printf("%c", distances[0].second[p]);
        p++;
    }
    dispImg = imread(distances[1].second);
    cv::imshow("Frame2", dispImg);
    printf("\nMatch #2:");
    p = 0;
    while (distances[1].second[p] != '\0')
    {
        printf("%c", distances[1].second[p]);
        p++;
    }
    dispImg = imread(distances[2].second);
    cv::imshow("Frame3", dispImg);
    printf("\nMatch #3:");
    p = 0;
    while (distances[2].second[p] != '\0')
    {
        printf("%c", distances[2].second[p]);
        p++;
    }
    dispImg = imread(distances[3].second);
    cv::imshow("Frame4", dispImg);
    printf("\nMatch #4:");
    p = 0;
    while (distances[3].second[p] != '\0')
    {
        printf("%c", distances[3].second[p]);
        p++;
    }
    dispImg = imread(distances[4].second);
    cv::imshow("Frame5", dispImg);
    printf("\nMatch #5:");
    p = 0;
    while (distances[4].second[p] != '\0')
    {
        printf("%c", distances[4].second[p]);
        p++;
    }
    dispImg = imread(distances[5].second);
    cv::imshow("Frame6", dispImg);
    printf("\nMatch #6:");
    p = 0;
    while (distances[5].second[p] != '\0')
    {
        printf("%c", distances[5].second[p]);
        p++;
    }
    dispImg = imread(distances[6].second);
    cv::imshow("Frame7", dispImg);
    printf("\nMatch #7:");
    p = 0;
    while (distances[6].second[p] != '\0')
    {
        printf("%c", distances[6].second[p]);
        p++;
    }
    dispImg = imread(distances[7].second);
    cv::imshow("Frame8", dispImg);
    printf("\nMatch #8:");
    p = 0;
    while (distances[7].second[p] != '\0')
    {
        printf("%c", distances[7].second[p]);
        p++;
    }
    dispImg = imread(distances[8].second);
    cv::imshow("Frame9", dispImg);
    printf("\nMatch #9:");
    p = 0;
    while (distances[8].second[p] != '\0')
    {
        printf("%c", distances[8].second[p]);
        p++;
    }
    dispImg = imread(distances[9].second);
    cv::imshow("Frame10", dispImg);
    printf("\nMatch #10:");
    p = 0;
    while (distances[9].second[p] != '\0')
    {
        printf("%c", distances[9].second[p]);
        p++;
    }
    while (cv::waitKey(1) != 'q');
    cv::destroyAllWindows();
    closedir(dirp);
    return 0;
}

int main()
{
    int tasknum;
    printf("\nTask #?Enter 1/2/3/4/5:");
    cin >> tasknum;
    printf("\n\n");
    if (tasknum == 1)
        task1();
    else if (tasknum == 2)
        task2();
    else if (tasknum == 3)
        task3();
    else if (tasknum == 4)
        task4();
    else if (tasknum == 5)
        task5();
    return 0;
}