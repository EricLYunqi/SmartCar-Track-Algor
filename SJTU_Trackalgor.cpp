#include <opencv2\core\core.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int my_sqrt(int x)
{
    int ans = 0, p = 128;

    while(p != 0)
    {
        ans += p;
        if(ans * ans > x)
            ans -= p;
        p >>= 1;
    }

    return ans;
}

int my_max(int a,int b)
{
    return a > b ? a : b;
}

int my_min(int a,int b)
{
    return a < b ? a : b;
}

//最小二乘法

//赛道曲率计算
float process_curvity(int x1, int y1, int x2, int y2, int x3, int y3)
{
    float K;
    int square_of_ABC = ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) >> 1;

    int q1 = ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    int AB = my_sqrt(q1);
    q1 = ((x3 - x2) * (x3 - x2) + (y3 - y2) * (y3 - y2));
    int BC = my_sqrt(q1);
    q1 = ((x3 - x1) * (x3 - x1) + (y3 - y1) * (y3 - y1));
    int AC = my_sqrt(q1);

    if(AB * BC * AC == 0)
        K = 0;
    else
        K = (float)4 * square_of_ABC / (AB * BC * AC);

    return K;
}

int main()
{
    cout << "Hello Opencv4.5" << endl;

    Mat track1;

    track1 = imread("H:/C++programs/image/track1.png",IMREAD_GRAYSCALE);
    if (track1.empty())
	{
		printf("could not load image...\n");
		return -1;
	}

    int hight = track1.rows;
    int width = track1.cols;

    //2*2最小池化
    Mat track2 = Mat::zeros(Size(width/2, hight/2), CV_8UC1);

    for(int i = 1; i < hight; i += 2)
    {
        for(int j = 1; j < width; j += 2)
        {
            int min_value = 255;
            if(track1.at<uchar>(i,j) < min_value) 
                min_value = track1.at<uchar>(i,j);
            if(track1.at<uchar>(i-1,j) < min_value) 
                min_value = track1.at<uchar>(i,j);
            if(track1.at<uchar>(i,j-1) < min_value) 
                min_value = track1.at<uchar>(i,j);
            if(track1.at<uchar>(i-1,j-1) < min_value) 
                min_value = track1.at<uchar>(i,j);
            track2.at<uchar>(i/2, j/2) = min_value;
        }
    }

    //初步寻线
    Mat track3 = Mat::zeros(Size(width/2, hight/2), CV_8UC1);
    int dir_front[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    int dir_frontleft[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
    int dir_frontright[4][2] = {{1, -1}, {1, 1}, {-1, 1}, {-1, -1}};

    int left_line[10000][2] = {0};
    int right_line[10000][2] = {0};
    int middle_line[10000][2] = {0};
    int left_line_length = 0, right_line_length = 0;

    int hight2 = hight / 2;
    int width2 = width / 2;

    //寻找起点
    int start = width2 / 2;
    int start_left = 0;
    int start_right = 0;

    for(int i = 0; i < width2 / 2 - 2; i++)
    {
        int left = start - i;
        int right = start + i;

        // if(track2.at<uchar>(hight2 - 5,left) == track2.at<uchar>(hight2 - 5,left - 1) ||
        //     track2.at<uchar>(hight2 - 5,right) == track2.at<uchar>(hight2 - 5,right + 1))
        //     continue;

        // float left_minus_div_sum = (track2.at<uchar>(hight2 - 5,left) - track2.at<uchar>(hight2 - 5,left - 1)) 
        //                             / (track2.at<uchar>(hight2 - 5,left) + track2.at<uchar>(hight2 - 5,left - 1));
        // float right_minus_div_sum = (track2.at<uchar>(hight2 - 5,right) - track2.at<uchar>(hight2 - 5,right + 1)) 
        //                             / (track2.at<uchar>(hight2 - 5,right) + track2.at<uchar>(hight2 - 5,right + 1));

        //cout << left_minus_div_sum << " " << right_minus_div_sum << endl;

        int x = track2.at<uchar>(hight2 - 10,left);
        int y = track2.at<uchar>(hight2 - 10,right);
        int left_x = track2.at<uchar>(hight2 - 10,left - 1);
        int right_y = track2.at<uchar>(hight2 - 10,right + 1);
        int flag_left = 0;
        int flag_right = 0;

        int left_minus_div_sum = (x - left_x) * 1000 / (x + left_x);
        int right_minus_div_sum = (y - right_y) * 1000 / (y + right_y);

        //cout << x - left_x << " " << x + left_x << " " << y - right_y << " " << y + right_y << endl;

        if(left_minus_div_sum > 200 && left_x < 50 && flag_left == 0)
        {
            start_left = left;
            flag_left = 1;
        }
        if(right_minus_div_sum > 200 && right_y < 50 && flag_right == 0)
        {
            start_right = right;
            flag_right = 1;
        }

        if(flag_left == 1 && flag_right == 1)
            break;
    }

    for(int i = 10; i < hight2 - 1; i++)
    {
        track2.at<uchar>(hight2 - i, start_left) = 0;
        track2.at<uchar>(hight2 - i, start_right) = 0;
    }

    int x1 = track2.at<uchar>(hight2 - 10,start_right);
    int x2 = track2.at<uchar>(hight2 - 10,start_right-1);
    int x3 = track2.at<uchar>(hight2 - 10,start_right+1);
    cout << x1 << " " << x2 << " " << x3 << endl;
    cout << start_right << endl;

    //左手寻线
    int num = hight2 + 70;
    int block_size = 7;
    int clip_value = 3;
    int half = block_size / 2;
    int step = 0, dir = 0, turn = 0;

    int x = start_left;
    int y = hight2 - 10;

    while(step < num && half < x && x < width2 - half - 1 && half < y && y < hight2 - half - 1 && turn < 4)
    {
        int local_thres = 0;
        for(int dx = -half; dx <= half; dx++)
            for(int dy = -half; dy <= half; dy++)
                local_thres += track2.at<uchar>(y + dy, x + dx);
        local_thres /= block_size * block_size;
        local_thres -= clip_value;

        int current_value = track2.at<uchar>(y, x);
        int front_value = track2.at<uchar>(y + dir_front[dir][1], x + dir_front[dir][0]);
        int frontleft_value = track2.at<uchar>(y + dir_frontleft[dir][1], x + dir_frontleft[dir][0]);
        if(front_value < local_thres)
        {
            dir = (dir+1) % 4;
            turn ++;
        }
        else if(frontleft_value < local_thres)
        {
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            left_line[left_line_length++][0] = x;
            left_line[left_line_length][1] = y;

            //track3.at<uchar>(y, x) = 255;
            step ++;
            turn = 0;
        }
        else
        {
            x += dir_frontleft[dir][0];
            y += dir_frontleft[dir][1];
            left_line[left_line_length++][0] = x;
            left_line[left_line_length][1] = y;

            //track3.at<uchar>(y, x) = 255;
            dir = (dir + 3) % 4;
            step ++;
            turn = 0;
        }
    }

    //右手巡线
    num = hight2 + 70;
    block_size = 7;
    clip_value = 3;
    half = block_size / 2;
    step = 0, dir = 0, turn = 0;

    x = start_right;
    y = hight2 - 10;

    while(step < num && 0 < x && x < width2 - 1 && 0 < y && y < hight2 - 1 && turn < 4)
    {
        int local_thres = 0;
        for(int dx = -half; dx <= half; dx++)
            for(int dy = -half; dy <= half; dy++)
                local_thres += track2.at<uchar>(y + dy, x + dx);
        local_thres /= block_size * block_size;
        local_thres -= clip_value;

        int current_value = track2.at<uchar>(y, x);
        int front_value = track2.at<uchar>(y + dir_front[dir][1], x + dir_front[dir][0]);
        int frontright_value = track2.at<uchar>(y + dir_frontright[dir][1], x + dir_frontright[dir][0]);
        if(front_value < local_thres)
        {
            dir = (dir+3) % 4;
            turn ++;
        }
        else if(frontright_value < local_thres)
        {
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            right_line[right_line_length++][0] = x;
            right_line[right_line_length][1] = y;

            //cout << x << " " << y << endl;
            //track3.at<uchar>(y, x) = 255;
            step ++;
            turn = 0;
        }
        else
        {
            x += dir_frontright[dir][0];
            y += dir_frontright[dir][1];
            right_line[right_line_length++][0] = x;
            right_line[right_line_length][1] = y;

            //cout << x << " " << y << endl;
            //track3.at<uchar>(y, x) = 255;
            dir = (dir + 1) % 4;
            step ++;
            turn = 0;
        }
    }

    //均值滤波算法
    for(int i = 2; i < left_line_length - 2; i++)
    {
        left_line[i][0] = (left_line[i-2][0] + left_line[i-1][0] + left_line[i][0] + left_line[i+1][0] + left_line[i+2][0]) / 5;
        left_line[i][1] = (left_line[i-2][1] + left_line[i-1][1] + left_line[i][1] + left_line[i+1][1] + left_line[i+2][1]) / 5;

        right_line[i][0] = (right_line[i-2][0] + right_line[i-1][0] + right_line[i][0] + right_line[i+1][0] + right_line[i+2][0]) / 5;
        right_line[i][1] = (right_line[i-2][1] + right_line[i-1][1] + right_line[i][1] + right_line[i+1][1] + right_line[i+2][1]) / 5;
    }

    for(int i = 0; i < left_line_length; i++)
    {
        int y = left_line[i][1];
        int x = left_line[i][0];
        track3.at<uchar>(y, x) = 255;
    }
    for(int i = 0; i < right_line_length; i++)
    {
        int y = right_line[i][1];
        int x = right_line[i][0];
        track3.at<uchar>(y, x) = 255;
    }

    //拟合中线
    int middle_line_length = my_min(left_line_length, right_line_length);

    for(int i = 10; i < middle_line_length; i++)
    {
        middle_line[i][0] = (left_line[i][0] + right_line[i][0]) >> 1;
        middle_line[i][1] = (left_line[i][1] + right_line[i][1]) >> 1;
        track3.at<uchar>(middle_line[i][1], middle_line[i][0]) = 255;
    }


    imshow("test",track3);
    //imshow("test",track2);
    waitKey(0);

    cout << "hello world" << endl;
    return 0;
}
