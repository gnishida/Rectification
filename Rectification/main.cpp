#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <glm/glm.hpp>

#ifndef SQR
#define SQR(x)		((x) * (x))
#endif

std::vector<cv::Point2f> order_points(const std::vector<cv::Point2f>& pts) {
	// initialzie a list of coordinates that will be ordered
	// such that the first entry in the list is the top - left,
	// the second entry is the top - right, the third is the
	// bottom - right, and the fourth is the bottom - left
	std::vector<cv::Point2f> rect(4);

	// the top - left point will have the smallest sum, whereas
	// the bottom - right point will have the largest sum
	{
		float min_sum = std::numeric_limits<float>::max();
		int min_id;
		float max_sum = -std::numeric_limits<float>::max();
		int max_id;
		for (int i = 0; i < pts.size(); ++i) {
			if (pts[i].x + pts[i].y < min_sum) {
				min_sum = pts[i].x + pts[i].y;
				min_id = i;
			}
			if (pts[i].x + pts[i].y > max_sum) {
				max_sum = pts[i].x + pts[i].y;
				max_id = i;
			}
		}
		rect[0] = pts[min_id];
		rect[2] = pts[max_id];
	}

	// now, compute the difference between the points, the
	// top - right point will have the smallest difference,
	// whereas the bottom - left will have the largest difference
	{
		float min_sum = std::numeric_limits<float>::max();
		int min_id;
		float max_sum = -std::numeric_limits<float>::max();
		int max_id;
		for (int i = 0; i < pts.size(); ++i) {
			if (pts[i].x - pts[i].y < min_sum) {
				min_sum = pts[i].x - pts[i].y;
				min_id = i;
			}
			if (pts[i].x - pts[i].y > max_sum) {
				max_sum = pts[i].x - pts[i].y;
				max_id = i;
			}
		}
		rect[1] = pts[max_id];
		rect[3] = pts[min_id];
	}

	// return the ordered coordinates
	return rect;
}

cv::Mat four_point_transform(cv::Mat& image, const std::vector<cv::Point2f>& pts) {
	// obtain a consistent order of the points and unpack them
	// individually
	std::vector<cv::Point2f> rect = order_points(pts);

	cv::Point2f tl = rect[0];
	cv::Point2f tr = rect[1];
	cv::Point2f br = rect[2];
	cv::Point2f bl = rect[3];

	// compute the width of the new image, which will be the
	// maximum distance between bottom - right and bottom - left
	// x - coordiates or the top - right and top - left x - coordinates
	float widthA = cv::norm(br - bl);
	float widthB = cv::norm(tr - tl);
	int maxWidth = std::max(int(widthA), int(widthB));

	// compute the height of the new image, which will be the
	// maximum distance between the top - right and bottom - right
	// y - coordinates or the top - left and bottom - left y - coordinates
	float heightA = cv::norm(tr - br);
	float heightB = cv::norm(tl - bl);
	int maxHeight = std::max(int(heightA), int(heightB));

	// now that we have the dimensions of the new image, construct
	// the set of destination points to obtain a "birds eye view",
	// (i.e. top-down view) of the image, again specifying points
	// in the top - left, top - right, bottom - right, and bottom - left
	// order
	std::vector<cv::Point2f> dst(4);
	dst[0] = cv::Point2f(0, 0);
	dst[1] = cv::Point2f(maxWidth - 1, 0);
	dst[2] = cv::Point2f(maxWidth - 1, maxHeight - 1);
	dst[3] = cv::Point2f(0, maxHeight - 1);

	// compute the perspective transform matrix and then apply it
	cv::Mat M = cv::getPerspectiveTransform(rect, dst);
	cv::Mat warped;
	cv::warpPerspective(image, warped, M, cv::Size(maxWidth, maxHeight));

	// return the warped image
	return warped;
}

int main() {
	cv::Mat img = cv::imread("image.png");
	std::vector<cv::Point2f> pts;
	pts.push_back(cv::Point2f(489, 437));
	pts.push_back(cv::Point2f(716, 548));
	pts.push_back(cv::Point2f(497, 596));
	pts.push_back(cv::Point2f(711, 707));

	cv::Mat warped = four_point_transform(img, pts);
	cv::imwrite("output.png", warped);

	return 0;
}