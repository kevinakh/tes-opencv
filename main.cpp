// opencv
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Windows.h"

// C
#include <stdio.h>
// C++
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;


// Function Headers
void createTrackbars();
string intToString(int number);
void writeScreen(Mat &screen, string String);
void MoveMouse(LONG X, LONG Y);
void DoMouseClick();
void ClickPressed();
void ReleaseClick();
void convertSize (int &x, int &y);
void writeDesc(Mat &screen2);
void center_circle();


//				Variables
// Matrix variables
Mat frame, ycrcb, Output;

//initial min and max ycrcb filter values.
//these will be changed using trackbars
int Y_MIN = 0;
int Y_MAX = 255;
int Cb_MIN = 131;
int Cb_MAX = 185;
int Cr_MIN = 80;
int Cr_MAX = 135;
int THRESH_MIN = 0;
int THRESH_MAX = 255;
int jari1 = 1;
int jari5 = 5;
int kiri =1;
int kanan = 1;
int tengah = 1;
int hitung = 2;
int jumlah;

//rectangle utk screen area buat mouse area
		int scrX = 10;
		int scrY = 10;
//for 3x times scale
		int scrW = 544;
		int scrH = 256;
		

// contours variables
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
int countDefects = 0;
bool klikmouse = false;
//names that will appear at the top of each window
const string trackbarWindowName = "Trackbars";
void on_trackbar2( int, void*)
{
	
}

int main( int argc, char* arg[] )
{
	// Set video source
	VideoCapture cap(0);

	int key;
	RNG rng(12345);

	if(!cap.isOpened()){

		cout << "Error"; return -1;
	}
	namedWindow(trackbarWindowName);
	createTrackbars();
	//tarckbar jari
	namedWindow( "Konfigurasi Inputan Mouse", 1 );
	createTrackbar("Klik kiri", "Konfigurasi Inputan Mouse", &kiri, 5,on_trackbar2 );
	createTrackbar("Klik kanan", "Konfigurasi Inputan Mouse", &kanan, 5,on_trackbar2 );
	createTrackbar("Scroll", "Konfigurasi Inputan Mouse", &tengah, 5, on_trackbar2 );
	
	while(true){
		// Get frame from the screen
		cap.read(frame);
		flip(frame,frame,1);

		// Convert it to ycrcb format
		cvtColor(frame, ycrcb, COLOR_BGR2YCrCb);

		// Filter ycrcb image between values and store it in other matrix
		inRange( ycrcb, Scalar( Y_MIN, Cb_MIN, Cr_MIN ), Scalar( Y_MAX, Cb_MAX, Cr_MAX ),
				Output);

		// Detect edges using Threshold
		threshold( Output, Output, THRESH_MIN, THRESH_MAX, THRESH_BINARY );

		// Find contours
		findContours( Output.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE,
						Point( 0, 0 ) );

		int largestPos = 0;		// Largest contour position
		int largestArea = 0;	// largestArea of this contour
		int x,y;				// largest contour's x and y coordinate

		vector< vector<Point> > hull( contours.size());
		vector< vector<int> > hullsI( contours.size());
		vector< vector<Vec4i> >defects(contours.size());
		vector<Point2f>center( contours.size() );
		vector<vector<Point> > contours_poly( contours.size() );
		vector<Point2f> lingkar(contours.size());
		
	
		// Draw contours
		Mat drawing = Mat::zeros( Output.size(), CV_8UC3 );
		
		for( int i = 0; i < contours.size(); i++ )
		{
			Moments moment = moments( (Mat)contours[i] );
			double area = moment.m00;

			convexHull( Mat(contours[i]), hull[i], false );
			convexHull( Mat(contours[i]), hullsI[i], false );

			// If major area and major then minimum required
			if( area > largestArea)
			{
				largestArea = area;
				// Get x,y coordinates
				x = moment.m10 / area;
				y = moment.m01 / area;

				largestPos = i;
			}
			//minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
			

		}
		
		//Ini utk ngatasi Error assertion failed
		cout << "Area: " << largestArea << endl;
		
		// NEW PART HERE
		if( largestArea < 1000 ){
			// Show Original and Converted images
			imshow("Normal", frame);
			imshow("Output", Output);
			imshow("Contours", drawing);
			imshow("YcrCB",ycrcb);

			// Get a key pressed by user
			key = waitKey( 30 );
			if ((char)key == 'q' || (char)key == 27) break; // If 'ESC' or 'q' then exit

			continue;
		}// END OF NEW PART HERE


		// Dapatkan Hull dari Contour kita
		convexHull( Mat(contours[largestPos]), hull[largestPos], false );
		convexityDefects(Mat(contours[largestPos]),hullsI[largestPos], defects[largestPos]);
		//Create centroid points in palm area
		//Create a moment variable 
		Moments mu;
		mu = moments( contours[largestPos], false );
		/***************************************/
		// Get mass center 
		int i=0;
		Point2f mc = Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
		
		circle (drawing, mc,8,Scalar(0,0,255),-1,8,0);
		circle (drawing, mc,64,Scalar(0,255,255),2,8,0);
		circle (frame, mc,8,Scalar(0,0,255),-1,8,0);
		circle (frame, mc,64,Scalar(0,255,255),2,8,0);

		/*					HERE WE BUILD THE RECTANGLE									*/

		// Build a rectangle around the largest hull ( dont draw it yet )
		vector<Point> contour_poly;
		Rect boundRect;
		approxPolyDP( Mat(contours[largestPos]), contour_poly, 3, true );
		boundRect = boundingRect( Mat(contour_poly) );
		//start point for the screen 
		Point screen_point = Point(scrX,scrY);
		//width and height of the screen
		Size screen_size = Point(scrW,scrH);
		Rect layar = Rect(screen_point,screen_size);
		rectangle(frame,layar, Scalar(0,0,0),2);

		//PENGGAMBARAN SHAPES
		
		//Draw Line in the center
		int Rectx = boundRect.x;
		int Recty = boundRect.y;
		int width = boundRect.width;
		int height = boundRect.height;
		int offset = -20;
		int Batas_defects = Recty+height/2+ offset;
		boundRect.height = 350;
		rectangle( drawing, boundRect.tl(), boundRect.br(), Scalar(255,0,255), 2, 8, 0 );
		//line in the middle:
		line( drawing, Point( Rectx, Recty + boundRect.height/2-(offset-100)), Point( Rectx + boundRect.width, Recty + boundRect.height/2-(offset-100)), Scalar(0,255,255), 2 );
		//circle in the middle
		//center_circle();
		int titikx = Rectx+(width/2);
		int titiky = Recty+(boundRect.height/2);
		Point lingkaran = Point(titikx,titiky);
		//circle( drawing, lingkaran, width/4 , Scalar( 0, 0, 255 ), 2, 8 );
		//circle(drawing, lingkaran, width/64, Scalar(0,0,255),-1);
		//circle( frame, lingkaran, width/4, Scalar( 0, 0, 255 ), 2, 8 );
		cout << "Height hand: " <<  height << endl;
		cout << "Width hand: " <<  width << endl;
		/*											END							*/
		//END PENGGAMBARAN SHAPE
		//int Batas_contour = boundRect.height/2-(offset-100);
		//if( y < Batas_contour){
		
		//window output
		drawContours( drawing, contours, largestPos, Scalar(255,0,0), 1, 8, vector<Vec4i>(),
				0, Point() );
		
		//window normal
		drawContours( frame, contours, largestPos, Scalar(255,0,0), 3, 8, vector<Vec4i>(),
				0, Point() );

		 //Draw Hull
		//window output
		drawContours( drawing, hull, largestPos, Scalar(0,255,0), 1, 8, vector<Vec4i>(),0, Point());
		
		//window normal
		drawContours( frame, hull, largestPos, Scalar(0,255,0), 3, 8, vector<Vec4i>(),0, Point());
		// Get the defects
		
		size_t count = contours[largestPos].size();
		if( count > 300 ){
			vector<Vec4i>::iterator d =defects[largestPos].begin();
			while( d!=defects[largestPos].end() ) {
				Vec4i& v=(*d);

				int startidx=v[0];
				Point ptStart( contours[largestPos][startidx] ); // point of the contour where the defect begins
				int endidx=v[1];
				Point ptEnd( contours[largestPos][endidx] ); // point of the contour where the defect ends
				int faridx=v[2];
				Point ptFar( contours[largestPos][faridx] ); // the farthest from the convex hull point within the defect
				float depth = v[3] / 256; // distance between the farthest point and the convex hull

				cout << "Depth value: " << depth << endl;
				cout << "Height hand: " <<  boundRect.height << endl;
				cout << "Width hand: " <<  boundRect.width << endl;
				if(ptStart.y < Batas_defects){
				if(depth > 5 && depth < 180)
				{
					line( drawing, ptStart, ptFar, Scalar(0,255,0), 2 );
					line( drawing, ptEnd, ptFar, Scalar(0,255,0), 2 );
					circle( drawing, ptStart,   4, Scalar(100,0,255), 2 );
					circle( frame, ptStart,   4, Scalar(100,0,255), 5 );
					cout << "Depth value: " << depth << endl;
					//counting process
					//countDefects += 1;   // Counting Jari
					countDefects = countDefects+1;
					jumlah = hitung + 1;
				}
					
				}
				
				if (layar.contains ( lingkaran )){
							int x = lingkaran.x;
							int y = lingkaran.y;
							convertSize(x,y);
							cout << "Coordinate x: " << x <<endl;
							cout << "Coordinate y: " << y <<endl;
							if(countDefects == 1){
								
								MoveMouse(x,y);//move mouse berdasarkan titik x,y pada middle circle
								//ReleaseClick();
							}
							else if(countDefects == jumlah){
							    DoMouseClick();
								//ClickPressed();
								//cout << "Mouse telah mengklik sesuatu" << endl;
							}
							
					}

				d++;
				
			}
			
			writeScreen( frame, intToString(countDefects) );
			countDefects = 0;
			writeDesc(frame);

		}

		

		
		// Show Original and Converted images
		namedWindow("Normal",WINDOW_NORMAL);
		namedWindow("YcrCB",WINDOW_NORMAL);
		namedWindow("Output",WINDOW_NORMAL);
		namedWindow("Contours",WINDOW_NORMAL);
		imshow("Normal", frame);
		imshow("YcrCB", ycrcb);
		imshow("Output", Output);
		imshow("Contours", drawing);
		resizeWindow("Normal",640,480);
		resizeWindow("YcrCB",400,350);
		resizeWindow("Output",400,350);
		resizeWindow("Contours",400,350);

		// Get a key pressed by user
		key = waitKey( 30 );
		if ((char)key == 'q' || (char)key == 27) continue;
		// If 'ESC' or 'q' then exit
	}

}

void center_circle(){
	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect2( contours.size() );
	vector<Point2f>center( contours.size() );
	vector<float>radius( contours.size() );

  for( int i = 0; i < contours.size(); i++ )
     { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
       //boundRect2[i] = boundingRect2( Mat(contours_poly[i]) );
       minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
     }


  /// Draw polygonal contour + bonding rects + circles
  Mat drawing2 = Mat::zeros( Output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
       rectangle( drawing2, boundRect2[i].tl(), boundRect2[i].br(), Scalar(0,0,255), 2, 8, 0 );
       circle( drawing2, center[i], (int)radius[i], Scalar(0,0,255), 2, 8, 0 );
     }

}

//Storing Mouse Coordinate:
void on_mouse( int e, int x, int y, int d, void *ptr )
{
    Point*p = (Point*)ptr;
    p->x = x;
    p->y = y;
}

void convertSize (int &x, int &y){
	x = x - scrX;
	y = y - scrY;
	cout << "Valor x:" << x << endl;
	cout << "Valor y:" << y << endl;
	//sesuaikan resolusi layar disni:
	x = ((1366*2)*x)/scrW;
	y = (768*y)/scrH;

}

//Mouse Movement Function:
int ScreenWidth() { return GetSystemMetrics(SM_CXSCREEN) - 1; }
int ScreenHeight() { return GetSystemMetrics(SM_CYSCREEN) - 1; }
//int offset = 10000;

void MoveMouse(LONG x, LONG y)
{
    INPUT in = {
        INPUT_MOUSE,
        {
			x * (65535.0 / ScreenWidth()),  // Map the x,y position to absolute 
			y * (65535.0 / ScreenHeight()), // mouse coords in a [0,65535) range
            0, 
            MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE
        }
    };
    SendInput(1, &in, sizeof in);
}
//End of Mouse Movement Function

//this function will click the mouse using the parameters assigned to it

void Scroll_up(){
	MOUSEEVENTF_WHEEL;

}
void DoMouseClick()
        {
			INPUT zero = {INPUT_MOUSE, 0};
			INPUT in = zero;
			in.mi.dwFlags  = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, &in, sizeof in);
			in = zero;
			in.mi.dwFlags  = MOUSEEVENTF_LEFTUP;
			SendInput(1, &in, sizeof in);
			waitKey(10);
        }

void ClickPressed(){
			INPUT zero = {INPUT_MOUSE, 0};
			INPUT in = zero;
			in.mi.dwFlags  = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, &in, sizeof in);
			//waitKey(10);
}

void ReleaseClick()
{	
			INPUT zero = {INPUT_MOUSE, 0};
			INPUT in = zero;
			in.mi.dwFlags  = MOUSEEVENTF_LEFTUP;
			SendInput(1, &in, sizeof in);
			//waitKey(10);
}


// Convert from number to string
string intToString(int number)
{
	stringstream ss;
	ss << number;
	return ss.str();
}

void writeScreen(Mat &screen, string String){
	putText( screen, String, Point( 550, 100 ),
			1, 7, Scalar( 0,0,255), 5);
}

void writeDesc(Mat &screen2){
		String desc = "Layar Monitor Skala 1:5";
		putText(screen2, desc, Point(scrY,(scrY+scrH+40)),1,3,Scalar(0,0,255),5);
}

//This function gets called whenever a trackbar position is changed
void on_trackbar( int, void* )
{
}

void createTrackbars(  ){
	//create window for trackbars
	namedWindow(trackbarWindowName, CV_WINDOW_KEEPRATIO);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->
    createTrackbar( "Y_MIN", trackbarWindowName, &Y_MIN, Y_MAX, on_trackbar );
    createTrackbar( "Y_MAX", trackbarWindowName, &Y_MAX, Y_MAX, on_trackbar );
    createTrackbar( "Cb_MIN", trackbarWindowName, &Cb_MIN, Cb_MAX, on_trackbar );
    createTrackbar( "Cb_MAX", trackbarWindowName, &Cb_MAX, Cb_MAX, on_trackbar );
    createTrackbar( "Cr_MIN", trackbarWindowName, &Cr_MIN, Cr_MAX, on_trackbar );
    createTrackbar( "Cr_MAX", trackbarWindowName, & Cr_MAX, Cr_MAX, on_trackbar );
	
}
