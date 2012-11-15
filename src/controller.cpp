#include "controller.h"

void Controller::show_usage() {
  cout << "usage:" << endl;
  cout << "./sauce command" << endl;
  cout << "command = extract | train | test" << endl;
}

void Controller::generate(char* input, char* output, int width, int height, int h_stride, int v_stride) {
    vector<string> dirs = Controller::listdir(input);
    int n=0;
    for(string& s : dirs) {
      Mat image = imread(s);
      Mat g;
      Sobel(image, g, -1, 1, 1);
      g = abs(g);

      int best = 0;
      int x=0, y=0;
      for(int i=0; i<g.rows-height; i+=v_stride) {
        for(int j=0; j<g.cols-width; j+=h_stride) {
          Mat windowH = g.rowRange(i, i+height);
          Mat window = windowH.colRange(j, j+width);
          Scalar windowSum = sum(window);
          if(windowSum(0) > best) {
            best = windowSum(0);
            x = j;
            y = i;
          }
        }
      }

      Mat finalWindowH = image.rowRange(y, y+height);
      Mat finalWindow = finalWindowH.colRange(x, x+width);
      char buf[1024];
      sprintf(buf, "%s/%4d.png", output, n++);
      imwrite(buf, finalWindow);
    }
}
  
void Controller::extract(Descriptor* desc, char* dir, char* output) {
    Mat features(0, 0, CV_32FC1);
    vector<string> dirs = Controller::listdir(dir);
    for(string& s : dirs) {
      Acc acc;
      acc.m = imread(s);
      Acc resultAcc = desc->iterate(acc);
      Controller::addFeature(features, resultAcc.m);
    }
    FileStorage fs(output, FileStorage::WRITE);
    fs << "features" << features;
}

void Controller::train(char* pos, char* neg, char* output) {
    FileStorage positives(pos, FileStorage::READ);
    FileStorage negatives(neg, FileStorage::READ);
    Mat posM; positives["features"] >> posM;
    Mat negM; negatives["features"] >> negM;
    Mat features(posM.rows+negM.rows, posM.cols, posM.type());
    Mat labels = Mat::zeros(posM.rows+negM.rows, 1, CV_32FC1);

    for(int i=0; i<posM.rows; i++) {
      posM.row(i).copyTo(features.row(i));
      labels.at<float>(i, 0) = 1;
    }
    for(int j=0; j<negM.rows; j++) {
      negM.row(j).copyTo(features.row(posM.rows+j));
    }

    CvSVM svm; CvSVMParams params;
    params.kernel_type = CvSVM::LINEAR;
    svm.train(features, labels, Mat(), Mat(), params);
    svm.save(output);
}

void Controller::predict(Descriptor* desc, char* set, char* model) {
  CvSVM svm;
  svm.load(model);

  int zeros=0, ones=0;
  vector<string> dirs = listdir(set);
  for(string& s : dirs) {
    Acc acc;
    acc.m = imread(s);
    Acc resultAcc = desc->iterate(acc);
    int predict = svm.predict(resultAcc.m.t());
    if(predict == 0) {
      zeros++;
    }
    else {
      ones++;
    }
  }
  int total = ones+zeros;
  cout << "Zeros: " << zeros << " (" << (float)(zeros)/total << ")" << endl;
  cout << "Ones: " << ones << " (" <<  (float)ones/total << ")" << endl;
  cout << "Total: " << total << endl;
}

auto Controller::addFeature(Mat& features, Mat s) -> void {
  if(s.rows == 0 && s.cols == 0) {
    features = s;
  }
  else {
    features.push_back(s);
  }
};

auto Controller::listdir(const char* path) -> vector<string>{
  DIR *pdir = NULL;
  pdir = opendir(path);
  struct dirent *pent = NULL;

  vector<string> files;
  while(pent = readdir(pdir)) {
    string file(pent->d_name);
    if(file!="." && file!="..") {
      file = "/"+file;
      files.push_back(path + file);
    }
  }
  closedir(pdir);
  return files;
};
