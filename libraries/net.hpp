
#include "eigen3/Eigen/Eigen"
#include "eigen3/Eigen/StdVector"
#include <iostream>
#include <vector>
#include "data.hpp"

using namespace std;



typedef double Number;
typedef Eigen::MatrixXf Matrix;
typedef Eigen::RowVectorXf RowVector;
typedef Eigen::VectorXf ColVector;

struct Layer{
  int type = 0; // 1 - first layer; 2 - hidden layer; 3 - last layer
  int size = 0;

  //ColVector input;
  ColVector output;
  ColVector bias;
  ColVector momentumBias;
  ColVector preOutput;

  Matrix deltas;
  Matrix outputCache;
  Matrix weights;
  Matrix momentumWeights;

  Layer(int t, int s){
    type = t;
    size = s;
  }
  void init(Layer &prevLayer);
  void feedForward(ColVector &input);
  void backPropagate(Layer &nextLayer);
  void lastLayerBackPropagate(ColVector &result, float &error);
  void updateWeights(Layer &prevLayer, const float learningRate, const float momentum, const int batchSize);
};

class Net{
  std::vector<Layer> layers;
  std::vector<int> topology;
  float learningRate;
  float momentum;
  int batchesSize;
  float error;
  string modelName;

public:
Net(string name, std::vector<int> t, float l, float m, int b){
    modelName = name;
    topology = t;
    learningRate = l;
    momentum = m;
    batchesSize = b;
    error = 0.0;
    init();
}
Net(string name, float l, float m, int b){
    modelName = name;
    learningRate = l;
    momentum = m;
    batchesSize = b;
    error = 0.0;
}
Net(string name){
  modelName = name;
};
void init(void);
void feedForward(ColVector &input);
void backPropagate(ColVector &result);
void updateWeights(const int batchSize);
void train(int epoches, Dataset data);
void logNetwork(void);
void readTopology();
void readNetwork();
vector<double> getResult(vector<double> input);
};
