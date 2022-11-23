#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat preprocess_image(Mat img) {
	Mat gray_image, blurred_image, canny_image, dilated_image;
	cvtColor(img, gray_image, COLOR_BGR2GRAY);
	GaussianBlur(gray_image, blurred_image, Size(3, 3), 3, 0);
	Canny(blurred_image, canny_image, 25, 75);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(canny_image, dilated_image, kernel);
	return dilated_image;
}

vector<Point> get_corner_points(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>> polygon_approx_contour(contours.size());

	vector<Point> biggest_element;
	int max_area = 0;

	for (int i = 0; i < contours.size(); i++) {
		int area = contourArea(contours[i]);

		if (area > 1000) {

			float perimeter = arcLength(contours[i], true);
			approxPolyDP(contours[i], polygon_approx_contour[i], 0.02 * perimeter, true);

			if (area > max_area && polygon_approx_contour[i].size() == 4) {

				biggest_element = { 
					polygon_approx_contour[i][0],
					polygon_approx_contour[i][1],
					polygon_approx_contour[i][2],
					polygon_approx_contour[i][3] 
				};

				max_area = area;

			}
		}
	}

	return biggest_element;
}

vector<Point> to_correct_order(vector<Point> points) {
	vector<Point> updated_points;
	vector<int> sum, difference;

	for (int i = 0; i < 4; i++) {
		sum.push_back(points[i].x + points[i].y);
		difference.push_back(points[i].x - points[i].y);
	}

	updated_points.push_back(points[min_element(sum.begin(), sum.end()) - sum.begin()]);
	updated_points.push_back(points[max_element(difference.begin(), difference.end()) - difference.begin()]);
	updated_points.push_back(points[min_element(difference.begin(), difference.end()) - difference.begin()]);
	updated_points.push_back(points[max_element(sum.begin(), sum.end()) - sum.begin()]);

	return updated_points;
}

Mat capture(Mat image, vector<Point> points, float width, float height) {

	Mat result;

	Point2f start[4] = {
	points[0],
	points[1],
	points[2],
	points[3]
	};

	Point2f end[4] = { {0.0f, 0.0f}, {width, 0.0f}, {0.0f, height}, {width, height} };

	Mat space_transform = getPerspectiveTransform(start, end);

	warpPerspective(image, result, space_transform, Point(width, height));

	return result;
}

void main() {
	string path = "Resources/document.jpg";
	Mat document_image = imread(path);

	Mat preprocessed_image = preprocess_image(document_image);

	vector<Point> corner_points = get_corner_points(preprocessed_image);

	vector<Point> ordered_corner_points = to_correct_order(corner_points);

	float width = 420, height = 690;

	Mat captured_document = capture(document_image, ordered_corner_points, width, height);

	//crop the edges
	Rect roi(5, 5, width - (2 * 5), height - (2 * 5));
	Mat cropped_image = captured_document(roi);


	imshow("CapturedDocument", captured_document);
	waitKey(0);

}
