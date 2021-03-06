clear all;
close all;

PositiveSetTrain = 'F:/inz/priv/sanity_test/train/pos';
NegativeSetTrain = 'F:/inz/priv/sanity_test/train/neg';

PositivesT = dir(PositiveSetTrain);

for i=3:length(PositivesT)
    ImageFile = [PositiveSetTrain, '/', PositivesT(i, 1).name];
    Image = imread(ImageFile);
    ImageGrayscale = rgb2gray(Image);
    
    [Gx, Gy] = hog_gradient(ImageGrayscale);
    [Magnitude, Orientation] = hog_mag_orient(Gx, Gy);
    
    Bins = hog_binning(9, Magnitude, Orientation);
    Bins = Bins / (norm(Bins, 1) + 0.001);
    
    Food(i-2, :) = Bins(:);
    Class(i-2, 1) = 1;
end

NegativesT = dir(NegativeSetTrain);
N = length(PositivesT)-2;
for i=3:length(NegativesT);
    ImageFile = [NegativeSetTrain, '/', NegativesT(i, 1).name];
    Image = imread(ImageFile);
    ImageGrayscale = rgb2gray(Image);
    
    [Gx, Gy] = hog_gradient(ImageGrayscale);
    [Magnitude, Orientation] = hog_mag_orient(Gx, Gy);
    
    Bins = hog_binning(9, Magnitude, Orientation);
    Bins = Bins / (norm(Bins, 1) + 0.001);
    
    Food(N + i-2, :) = Bins(:);
    Class(N + i-2, 1) = 0;
end
 
options = optimset('maxiter', 1000);
SVM=svmtrain(Food,Class,'quadprog_opts', options);
