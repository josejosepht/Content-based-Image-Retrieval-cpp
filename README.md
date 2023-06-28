# Content-based-Image-Retrieval-cpp
The project titled "Content-Based Image Retrieval" involves developing a system that takes a target image, an image database, a method to compute image features, a distance metric for comparing features, and the desired number of output images as inputs. The system then generates an ordered list of the most similar images to the target image based on the features and distance metric. The process consists of four steps: computing features for the target image, computing features for all images in the database, calculating the distance between the target image and all database images, and finally sorting and returning the top N matches. Optionally, the features can be computed offline and stored for efficient querying. The implementation can be a command line program or a GUI application. The tasks within the project include baseline matching, histogram matching, multi-histogram matching, texture and color analysis, and custom design for specific image types.

`sobelX3x3`: This function applies a 3x3 Sobel filter in the X-direction to detect horizontal edges in an input image.'x' key press for SobelX image in opencv window.

![image](https://github.com/josejosepht/Real-Time-Filtering-Cpp/assets/97187460/61331fe5-9380-41a1-b90d-0f87c75ab2db)


`sobelY3x3`: This function applies a 3x3 Sobel filter in the Y-direction to detect vertical edges in an input image.'y' key press for SobelY image in opencv window.

![image](https://github.com/josejosepht/Real-Time-Filtering-Cpp/assets/97187460/a41246cf-374c-4a8b-9a4e-8f15a1935f8c)


`magnitude`: This function computes the magnitude of gradient vectors at each pixel by combining the results from sobelX3x3 and sobelY3x3.'m' key press for magnitude image in opencv window.
![image](https://github.com/josejosepht/Real-Time-Filtering-Cpp/assets/97187460/064019fb-52d6-44b2-ba87-04f115a614b9)



[dirent.h for retrieving information about files and directories in C++](https://github.com/tronkko/dirent)

* filter.cpp and vidDisplay.cpp are to be placed in the Visual Studio project solution

* The implemented code for the filters on live video frames does not include a filter.h file as the filters are defined in filter.cpp inside the same Visual Studio 
project solution. The function prototypes declared at the top of vidDisplay.cpp call the required functions from filter.cpp
