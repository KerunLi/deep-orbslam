#include<iostream>
#include<algorithm>
#include<fstream>
#include<string>
#include<vector>
#include<iomanip>

#include<opencv2/core/core.hpp>
#include<opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define ImageWide 1241
#define ImageHigh 376

using namespace std;
using namespace cv;

void loadlines(const string &strPathToSequence, vector<vector<float>> &vLineEndPoints, const string &filename);
vector<size_t> getEndPointsInArea(const float &x, const float  &y, const float  &r, vector<vector<float>> &vLine2EndPoints, vector<bool> bmerged);
vector<vector<float>> mergedLines(vector<vector<float>> &vLineEndPoints);
float lineDistance(vector<float> &line1, vector<float> &line2);
vector<vector<float>> mergedLines(vector<vector<float>> &vLineEndPoints);

int main(int argc, char **argv)
{
    cerr << endl << "Usage: ./merge path_to_img1 path_to_img2 path_to_feature_folder" << endl;

    vector<vector<float>> lines1tmp,lines2tmp,matches12,mergedlines;
    loadlines(string(argv[3]), lines1tmp, string("/lines1.txt")); //y1,x1 y2,x2(h,w)
    loadlines(string(argv[3]), lines2tmp, string("/lines2.txt"));
    // loadlines(string(argv[3]), matches12, string("/matches.txt"));

    // cout<<lines1.size() <<"," << lines2.size()<<"," << matches12.size()<<endl;
    vector<vector<float>> lines,liness;
    for(int i=0;i<lines1tmp.size();i++){
        lines.push_back({lines1tmp[i][1],lines1tmp[i][0],lines1tmp[i][3],lines1tmp[i][2]});
        
    }
    for(int i=0;i<lines2tmp.size();i++){
        liness.push_back({lines2tmp[i][1],lines2tmp[i][0],lines2tmp[i][3],lines2tmp[i][2]});
        
    }
    

    vector<vector<float>> lines1 = lines;
    mergedlines = mergedLines(lines);
    
    Mat im1,im2,merged;
    im1 = imread(argv[1],IMREAD_UNCHANGED);
    im2 = imread(argv[2],IMREAD_UNCHANGED);
    cout <<im1.rows <<" "<<im1.cols<<endl;
    merged = imread(argv[1],IMREAD_UNCHANGED);
    
    cv::RNG rng(12);
    for (int i=0;i<lines1.size();i++){
        Point p1(lines1[i][1],lines1[i][0]);
        // cout << p1.x << " "<<p1.y <<endl;
        Point p2(lines1[i][3],lines1[i][2]);
        // cout << p2.x << " "<<p2.y <<endl;
        line(im1,p1,p2,Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255)),3,LINE_8);
    }
    for (int i=0;i<liness.size();i++){
        Point p1(liness[i][1],liness[i][0]);//x,y ->w,h
        // cout << p1.x << " "<<p1.y <<endl;
        Point p2(liness[i][3],liness[i][2]);
        // cout << p2.x << " "<<p2.y <<endl;
        line(im2,p1,p2,Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255)),3,LINE_8);
    }
    
    for (int i=0;i<mergedlines.size();i++){
        Point p1(mergedlines[i][1],mergedlines[i][0]);
        // cout << p1.x << " "<<p1.y <<endl;
        Point p2(mergedlines[i][3],mergedlines[i][2]);
        // cout << p2.x << " "<<p2.y <<endl;
        line(merged,p1,p2,Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255)),3,LINE_8);
    }
    
    imshow("Output1", im1);
    imshow("Output2", im2);
    
    imshow("Output3", merged);
    waitKey(0);


    return 0;

}


void loadlines(const string &strPathToSequence, vector<vector<float>> &vLineEndPoints, const string &filename){
    ifstream fTimes;
    string strPathTimeFile = strPathToSequence + filename;
    fTimes.open(strPathTimeFile.c_str());
    cout <<strPathTimeFile<<endl;

    while(!fTimes.eof())
    {
        string s;
        
        getline(fTimes,s);
        if (!s.empty()){
            stringstream ss;
            ss << s;
            vector<float> v;
            float element;

            int i = 0;
            while (ss >> element) {
                
                v.push_back(element);
            }
    
            vLineEndPoints.push_back(v);
        }
        
    }
    
    fTimes.close();
}

vector<size_t> getEndPointsInArea(const float &y, const float  &x, const float  &r, vector<vector<float>> &vLineEndPoints, vector<bool> bmerged){ //in the same one img
    vector<size_t> vIndices;
    float factorX = r;
    float factorY = r;

    const int nMinCellX = max(0,(int)(x-factorX));
    const int nMaxCellX = min(ImageWide-1,(int)(x+factorX));

    const int nMinCellY = max(0,(int)(y-factorY));
    const int nMaxCellY = min(ImageHigh-1,(int)(y+factorY));
    // cout <<nMinCellX <<" " <<nMaxCellX <<" "<<nMinCellY <<" "<<nMaxCellY<<endl;

    for (int i=0;i<vLineEndPoints.size();i++){
        if(vLineEndPoints[i][0]<nMaxCellY && vLineEndPoints[i][0]>nMinCellY && vLineEndPoints[i][1]<nMaxCellX && vLineEndPoints[i][1]>nMinCellX && !bmerged[i]){
            vIndices.push_back(i);
        }
        if(vLineEndPoints[i][2]<=nMaxCellY && vLineEndPoints[i][2]>=nMinCellY && vLineEndPoints[i][3]<=nMaxCellX && vLineEndPoints[i][3]>=nMinCellX && !bmerged[i]){
            vIndices.push_back(i);
        }
    }
    // cout << vIndices.size()<<endl;
    return vIndices;

}

vector<vector<float>> mergedLines(vector<vector<float>> &vLineEndPoints){ //call getEndPointsInArea() to get candicate
    vector<bool> bmerged(5*vLineEndPoints.size(),false);
    for (int i=0,iend=vLineEndPoints.size();i<iend;i++){
    // for (int i=0;i<vLineEndPoints.size();i++){
        vector<size_t> candicate1,candicate2;
        int y1 = vLineEndPoints[i][0];
        int x1 = vLineEndPoints[i][1];
        int y2 = vLineEndPoints[i][2];
        int x2 = vLineEndPoints[i][3];
        float yc = 0.5*(y1+y2);
        float xc = 0.5*(x1+x2);
        candicate1 = getEndPointsInArea(y1,x1,10,vLineEndPoints,bmerged);
        candicate2 = getEndPointsInArea(y2,x2,10,vLineEndPoints,bmerged);

        //compare distance and angle offset to decide if merge, is &&
        //averge to merge
        //the endpoint if be merged should be know
        //except i 
        vector<Point2d> points;
        points.push_back(Point2d(x1,y1));
        points.push_back(Point2d(x2,y2));
        for(int j=0;j<candicate1.size();j++){
            int id = candicate1[j];
            if (id!=i && !bmerged[id]){
                if (sqrt(norm((vLineEndPoints[id][0]-y1, vLineEndPoints[id][1]-x1)))<10 && lineDistance(vLineEndPoints[i],vLineEndPoints[id])<0.27){
                    // cout<<"norm:"<<norm((vLineEndPoints[id][0]-y1, vLineEndPoints[id][1]-x1))<<endl;
                    // cout<<"dis:"<<lineDistance(vLineEndPoints[i],vLineEndPoints[id])<<endl;
                    
                    points.push_back(Point2d(vLineEndPoints[id][1],vLineEndPoints[id][0]));
                    points.push_back(Point2d(vLineEndPoints[id][3],vLineEndPoints[id][2]));
                    
                    bmerged[i]=true;
                    bmerged[id]=true;

                }


            }
            
        }
        for(int j=0;j<candicate2.size();j++){
            int id = candicate2[j];
            if (id!=i && !bmerged[id]){
                if (sqrt(norm((vLineEndPoints[id][0]-y2, vLineEndPoints[id][1]-x2)))<10 && lineDistance(vLineEndPoints[i],vLineEndPoints[id])<0.27){
                    
                    points.push_back(Point2d(vLineEndPoints[id][1],vLineEndPoints[id][0]));
                    points.push_back(Point2d(vLineEndPoints[id][3],vLineEndPoints[id][2]));
                    
                    bmerged[i]=true;
                    bmerged[id]=true;

                }


            }
        }
        // cout <<points.size()<<endl;
        if(points.size()>2){
            // cout <<points.size()<<endl;
            cv::Vec4f line;
            fitLine(points,line, cv::DIST_L2, 0, 1e-2, 1e-2);

            cv::Point point0;
            point0.x = line[2];
            point0.y = line[3];
            float k = line[1] / line[0];

            float maxX1 = 0;
            float minX1 = 1241;
            // float maxY2 = 0;
            // float minY2 = 376;
            for (int j=0;j<points.size();j++){
                if(points[j].x>maxX1){
                    maxX1 = points[j].x;
                }
                
                if(points[j].x<minX1){
                    minX1 = points[j].x;
                }
                // if(points[j].y>maxY2){
                //     maxY2 = points[j].y;
                // }
                // if(points[j].y<minY2){
                //     minY2 = points[j].y;
                // }
                
            }
            cout<<maxX1<<","<<minX1<<endl;
            // cout << (points[1].y-point0.y)/(points[1].x-point0.x)<<","<<k<<endl;
            float maxY1 = k*(maxX1-point0.x)+point0.y;
            float minY1 = k*(minX1-point0.x)+point0.y;
            // float maxX2 = (maxY2-point0.y)/k+point0.x;
            // float minX2 = (minY2-point0.y)/k+point0.x;
            // float Y1 = 0.5*(maxY1+maxY2);
            // float X1 = 0.5*(maxX1+maxX2);
            // float Y2 = 0.5*(minY1+minY2);
            // float X2 = 0.5*(minX1+minX2);

            vLineEndPoints.push_back({maxY1,maxX1,minY1,minX1});
            // vLineEndPoints.push_back({Y1,X1,Y1,X1});
        }
        

    }
    
    vector<vector<float>> mergedLines;
    for (int i=0;i<vLineEndPoints.size();i++){
        // cout <<bmerged[i]<<endl;
        if(!bmerged[i]){
            mergedLines.push_back(vLineEndPoints[i]);
        }
    }
    mergedLines.push_back(vLineEndPoints.back());
    return mergedLines;
    // return vLineEndPoints;
}

float lineDistance(vector<float> &line1, vector<float> &line2){
    float line1Y1 = line1[0],line1X1 = line1[1],line1Y2 = line1[2],line1X2 = line1[3];
    float a1 = line1Y2-line1Y1;
    float b1 = line1X2-line1X1;
    float k1 = a1/b1;

    int line2Y1 = line2[0],line2X1 = line2[1],line2Y2 = line2[2],line2X2 = line2[3];
    float a2 = line2Y2-line2Y1;
    float b2 = line2X2-line2X1;
    float k2 = a2/b2;

    float dis = (float)fabs((k1-k2)/(1+k1*k2));
    return dis;
}