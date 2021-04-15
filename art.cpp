#include <bits/stdc++.h>
#include "util.h"
#include <jpeglib.h>
#define INF 1000007
#define ll long long

using namespace std;
template<class T>bool umin(T& a,T b){if(a>b){a=b;return 1;}return 0;}
template<class T>bool umax(T& a,T b){if(a<b){a=b;return 1;}return 0;}
default_random_engine gen;
/*
    Gray skaling and edge detection with Sobel kernel insipred from here
    https://opencv-python-tutroals.readthedocs.io/en/latest/py_tutorials/py_imgproc/py_canny/py_canny.html
*/ 
unsigned char *image;
int width, height, channels;

void SobelEdgeDetection(){
    if (channels == 3) { //RGB to gray scale (3 channels to 1 channel)
        unsigned char *image2; channels = 1;
        image2 = (unsigned char*) malloc(width * height * channels * sizeof(image));
        for (int i=0; i<height; i++) 
            for (int j=0; j<width; j++)
                image2[i*width+j] = (image[i*width*3+j*3] + image[i*width*3+j*3+1] + image[i*width*3+j*3+2])/3;
        free(image); image = image2;
    }

    int image2[height][width], A[height][width], B[height][width], C[height][width];

    for (int i=0; i<height; i++)
        for (int j=0; j<width; j++) 
            image2[i][j] = image[i*width+j];

    ///horizontal
    int mx = -INF, mn = INF;
    for (int i=1; i<height-1; i++){
        for (int j=1; j<width-1; j++) {
            A[i][j] = image2[i-1][j-1] + 2 * image2[i-1][j] + image2[i-1][j+1] -
                    image2[i+1][j-1] - 2 * image2[i+1][j] - image2[i+1][j+1];
            umin(mn, A[i][j]); umax(mx, A[i][j]);
        }
    }

    ///vertical
    mx = -INF; mn = INF;
    for (int i=1; i<height-1; i++){
        for (int j=1; j<width-1; j++) {
            B[i][j] = image2[i-1][j-1] + 2 * image2[i][j-1] + image2[i+1][j-1] -
                    image2[i-1][j+1] - 2 * image2[i][j+1] - image2[i+1][j+1];
            umin(mn, B[i][j]); umax(mx, B[i][j]);
        }
    }

    ///gradient
    mx = -INF; mn = INF;
    for (int i=0; i<height; i++) 
        for (int j=0; j<width; j++){
            C[i][j] = sqrt(pow(A[i][j], 2) + pow(B[i][j], 2));
            umin(mn, C[i][j]); umax(mx, C[i][j]);
        }

    for (int i=0; i<height; i++) 
        for (int j=0; j<width; j++)
            image[i*width+j] = ((C[i][j] - mn) / double(mx - mn) ) * 255;
    
}
/*
    Utility class for keeping image in 2D and in flatten version
    In case of needed delete from memory for optimization
*/
class artImage{ public:
    int H , W;
    vector < vector < int > > pixels;
    artImage(int _H , int _W){
        H = _H; W = _W;
        for(int i = 0 ; i < H ; i++){
            pixels.push_back(vector < int > ());
            pixels.back().resize(W);
            for(int j = 0 ; j < W ; j++)
                pixels[i][j] = 0;
        }
    }
    artImage(int _H , int _W, unsigned char *img){
        H = _H; W = _W;
        for(int i = 0 ; i < H ; i++){
            pixels.push_back(vector < int > ());
            pixels.back().resize(W);
            for(int j = 0 ; j < W ; j++)
                pixels[i][j] = img[i * W + j];
        }
    }
    artImage(artImage *img){
        H = img->H; W = img->W;
        for(int i = 0 ; i < H ; i++){
            pixels.push_back(vector < int > ());
            pixels.back().resize(W);
            for(int j = 0 ; j < W ; j++)
                pixels[i][j] = img->pixels[i][j];
        }
    }
    unsigned char* makeFlatten(){
        unsigned char *res;
        res = (unsigned char*) malloc(width * height * channels * sizeof(image));
        for(int j = 0 ; j < H ; j++)
            for(int i = 0 ; i < W ; i++)
                res[j * W + i] = pixels[j][i];
        return res;
    }
    void freeMemory(){
        for(auto &pp : pixels) pp.shrink_to_fit();
        pixels.shrink_to_fit();
    }
    ~artImage(){freeMemory();}

}*desired;
/*
    Fitness function returns the value how much far from final result
    F = sum(for all i,j: (cur[i][j]-desired)^2), smaller F much better
*/
ll sqr(int x){
    return x * 1LL * x;
}
ll getFitness(artImage *img){
    ll ret = 0;
    for(int r = 0 ; r < img->H ; r++)
        for(int c = 0 ; c < img->W ; c++)
            ret += sqr(img->pixels[r][c] - desired->pixels[r][c]);
    return ret;
}
/*
    Mutation function applied on single candiate
    Basically choosing some random circle with random radius
    with normal distributation respect to image pixel
*/
int alpha;
const int Generations = 3000;
const int populationSize = 5;
const int mutationProb = 3;
const int crosoverProb = 2;
artImage* mutation(artImage *img){
    uniform_int_distribution < int > CENTER (0 , max(height , width) - 1);
    uniform_int_distribution < int > RADIUS (min(width,height) >> 4 , min(width,height) >> 3);
    int X = min(CENTER(gen), width-1), Y = min(CENTER(gen), height-1), radius = RADIUS(gen);
    artImage *newImg = new artImage(img);
    for(int r = 0 ; r < img->H ; r++)
        for(int c = 0 ; c < img->W ; c++)
            if(sqr(c - X) + sqr(r - Y) <= sqr(radius)){
                normal_distribution < double > normalDis(desired->pixels[r][c] * 1.0 , alpha);
                newImg->pixels[r][c] = (int)(normalDis(gen));
                umin(newImg->pixels[r][c], 255);
                umax(newImg->pixels[r][c], 0);
            }
    if(getFitness(newImg) < getFitness(img)) swap(newImg, img);
    newImg->freeMemory(); delete(newImg);
    return img;
}
/*
    Crossover function takes two parents and creates child
    by swapping some pixels of parents in probability 1/crosoverProb
*/
artImage* crossover(artImage *father , artImage *mother){
    artImage *ret = new artImage(father);
    for(int j = 0 ; j < father->H ; j++)
        for(int i = 0 ; i < father->W ; i++)
            if(rand() % crosoverProb == 0)
                swap(father->pixels[j][i] , mother->pixels[j][i]);
    
    return ret;
}
/*
    Comparator for new population to sort population in increasing of fitness value
*/
bool cmp(artImage* a, artImage* b){
    return getFitness(a) < getFitness(b);
}
/*
    Genetic algorithm insight begins here
    Returns final image after all generations
*/
artImage *GeneticAlgorithm(){ 
    //Create Population with size of populationSize
    vector < artImage* > population;
    for(int j = 0 ; j < populationSize ; j++)
        population.push_back(new artImage(height , width));
    //Run the process desired number of generations
    for(int generation = 0 ; generation <= Generations ; generation++){
        vector < artImage* >Population;
        /*
            Select all possible pairs of population as parents and
            apply mutation for some of pairs in probability 1/mutationProb
        */
        for(int j = 0 ; j < populationSize; j++)
            for(int i = j + 1 ; i < populationSize ; i++){
                if(rand() % mutationProb == 0)
                    Population.push_back(mutation(crossover(population[j] , population[i])));
                else
                    Population.push_back(crossover(population[j] , population[i])); 
            }
        sort(Population.begin(), Population.end(), cmp);
        //Select the best fitness valued candidates for new population after crossover
        for(int j = 0 ; j < (int)Population.size(); j++){
            if(j < populationSize){
                population[j]->freeMemory(); delete(population[j]);
                population[j] = Population[j];
            }
            else  
                Population[j]->freeMemory(), delete(Population[j]);
        }
        if(generation % 10 == 0) {
            string path = "output/" + to_string(generation) + ".jpg";
            auto q = population[0]->makeFlatten();
            WriteJPEG(path, width, height, channels, q);
            if(generation % 300 == 0) alpha -= int(alpha*0.1); //Lose 10% of alpha in each 300th generation
        }
    }
    return population[0];
}
int main(int argc, char *argv[]){
    if (argc < 2) 
        return printf("ERROR[Specify input file];   Example: ./art input.jpg\n");
    //Clear output directory
    system("exec rm -r ./output/*");
    ReadJPEG(argv[1], &width, &height, &channels, &image);
    //Run edge detection algorithm to compute final picture
    SobelEdgeDetection();
    desired = new artImage(height , width , image);
    vector<int>cnt(256);
    for(int r = 0 ; r < desired->H ; r++)
        for(int c = 0 ; c < desired->W ; c++)
            cnt[desired->pixels[r][c]]++;
    //Detect alpha value for normal distribution
    for(int i = 0; i < 256; i++)
        if(cnt[i] > 256)
            alpha = i;
    //Run Genetic Algorithm and write final result in the end
    WriteJPEG("result_" + string(argv[1]) , width, height, channels, GeneticAlgorithm()->makeFlatten() );
    return 0;
}
