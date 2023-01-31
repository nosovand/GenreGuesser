#include "net.hpp"

#include <math.h>
#include <fstream>
#include <sstream>
#include <chrono>

float sigmoid(float x){
  return 1/(1+exp(-x));
}

float sigmoidDerivative(float x){
  if(isnan(sigmoid(x) * (1-sigmoid(x)))){
    clog << "Error sig" << endl;
    exit(0);
  }
  return sigmoid(x) * (1-sigmoid(x));
}

void Net::init(void){ //initializing network
  for (int l = 0; l < topology.size(); l++) {
    if(l == 0){ //creating layers
      Layer layer(1, topology.at(l));
      layer.init(layers.back());
      layers.push_back(layer);
    }
    if(l > 0 && l < topology.size()-1){
      Layer layer(2, topology.at(l));
      layer.init(layers.back());
      layers.push_back(layer);
    }
    if(l == topology.size()-1){
      Layer layer(3, topology.at(l));
      layer.init(layers.back());
      layers.push_back(layer);
    }
  }
}

void Layer::init(Layer &prevLayer){ //initializing layer
  output.resize(size); //according to the layer type, initialize vectors and matricies
  if(type != 3) outputCache = Eigen::MatrixXf::Constant(size, 0, 0.0);
  if(type > 1){
    bias = Eigen::VectorXf::Constant(size, 1.0);
    momentumBias = Eigen::VectorXf::Constant(size, 0.0);
    deltas = Eigen::MatrixXf::Constant(size, 0, 0.0);
    weights = Eigen::MatrixXf::Random(prevLayer.size, size);
    preOutput.resize(size);
    momentumWeights = Eigen::MatrixXf::Constant(prevLayer.size, size, 0.0);
  }
}

void Net::feedForward(ColVector &input){ //feeding foraward through whole network
  for (size_t l = 0; l < topology.size(); l++) {
    if(l == 0) layers[l].feedForward(input);
    else layers[l].feedForward(layers[l-1].output);
  }
}

void Layer::feedForward(ColVector &input){
  if(type == 1){ //first layer just sends input as output
    output = input;
  }
  else{ //other layers calculate outut sig(weigths * input + bias)
    preOutput = weights.transpose() * input + bias;
    output = preOutput.unaryExpr(&sigmoid);
  }
  if(type != 3) { //all layers except for the last one accumulate their outputs for weights update later
    outputCache.conservativeResize(outputCache.rows(), outputCache.cols()+1);
    outputCache.col(outputCache.cols()-1) = output;
  }
}

void Net::backPropagate(ColVector &result){ //calculate errors through whole network
  for (int l = topology.size() - 1; l > 0; l--) {
    if(l == topology.size() - 1){
      layers[l].lastLayerBackPropagate(result, error);
    }
    else{
      layers[l].backPropagate(layers[l+1]);
    }
  }
}

void Layer::lastLayerBackPropagate(ColVector &result, float &error){ //calculate last layer deltas (where we want to move, to reduce error)
  deltas.conservativeResize(deltas.rows(), deltas.cols()+1);
   deltas.col(deltas.cols()-1) = ((output - result).cwiseProduct(preOutput.unaryExpr(&sigmoidDerivative)));
   for (size_t i = 0; i < output.size(); i++) {
      error += (output[i] - result[i]) * (output[i] - result[i]);
   }
  //clog << "Output = " << endl << output << endl;
  //clog << "Result = " << endl << result << endl;
}

void Layer::backPropagate(Layer &nextLayer){ //calculate layer's error and add it to deltas matrix
  deltas.conservativeResize(deltas.rows(), deltas.cols()+1);
  deltas.col(deltas.cols()-1) = (nextLayer.weights * nextLayer.deltas.col(deltas.cols()-1)).cwiseProduct(preOutput.unaryExpr(&sigmoidDerivative));
}

void Net::updateWeights(const int batchSize){ //learning proccess
  //go through whole network and update weights
  for (size_t l = 1; l < topology.size(); l++) {
    layers[l].updateWeights(layers[l-1], learningRate, momentum, batchSize);
  }
}

void Layer::updateWeights(Layer &prevLayer, const float learningRate, const float momentum, const int batchSize){
  //create matrix containing change for every weight


  Matrix weightsDeltas = (prevLayer.outputCache * deltas.transpose()) * 1.0/(float)batchSize;

  //edit those changes with momentum of last change and learning rate
  momentumWeights = (learningRate * weightsDeltas + momentum * momentumWeights);

  //update weigths according to those changes
  weights = weights - momentumWeights;

  //update biases
  ColVector biasDeltas = Eigen::VectorXf::Constant(size, 0.0);
  for (size_t i = 0; i < deltas.cols(); i++) {
    biasDeltas += deltas.col(i);
  }
  momentumBias = learningRate * (biasDeltas * 1.0/(float)batchSize) + momentum * momentumBias;
  bias = bias - momentumBias;
  //delete cache
  deltas = Eigen::MatrixXf::Constant(size, 0, 0.0);
  prevLayer.outputCache = Eigen::MatrixXf::Constant(prevLayer.size, 0, 0.0);
}

void Net::logNetwork(void){

  clog << "Logging network parameters" << endl;
  ofstream outdata(modelName, std::ofstream::trunc);
  if(!outdata) cerr << "Error: file for logging network parameters could not be opened" << endl;
  typedef std::numeric_limits<float> fl;
  outdata.precision(fl::max_digits10);
  typedef std::numeric_limits<double> dbl;
  outdata.precision(dbl::max_digits10);
  for (size_t i = 0; i < topology.size(); i++) {
    outdata << topology[i] << " ";
  }
  outdata << endl;
  outdata << "<Net>" << endl;
  for (size_t l = 1; l < layers.size(); l++) {
    outdata << "<Layer>" << endl;
    outdata << l << endl;
    outdata << "<Bias>" << endl;
    outdata << layers[l].bias << endl;
    outdata << "</Bias>" << endl;
    outdata << "<MomentumBias>" << endl;;
    outdata << layers[l].momentumBias << endl;
    outdata << "</MomentumBias>" << endl;
    outdata << "<Weights>" << endl;
    outdata << layers[l].weights << endl;
    outdata << "</Weights>" << endl;
    outdata << "<MomentumWeights>" << endl;
    outdata << layers[l].momentumWeights << endl;
    outdata << "</MomentumWeights>" << endl;
    outdata << "</Layer>" << endl;
  }
  outdata << "</Net>" << endl;
}

void Net::readTopology(){
  ifstream file(modelName);
  if(!file){
    std::cerr << "Error: cannot open model file" << '\n';
  }
  topology.resize(0);
  string line;
  getline(file, line);
  int number;
  std::istringstream iss(line);
  while (iss >> number) {
    topology.push_back(number);
  }

}

void Net::readNetwork(){
  //loading pretrained model
  readTopology();
  init();
  ifstream file(modelName);
  string line;
  int layerNumber;
  double value;
  int iterator = 0;
  int iterator2 = 0;
  RowVector tmpVector;
  while (getline(file, line)) {
    if(line == "<Net>") {};
    if(line == "<Layer>"){
      getline(file, line);
      std::istringstream(line) >> layerNumber;
      while (line != "</Layer>") {
        getline(file, line);
        if (line == "<Bias>") {
          iterator = 0;
          while (iterator != layers[layerNumber].bias.size()) {
            getline(file, line);
            std::istringstream(line) >> value;
            layers[layerNumber].bias[iterator] = value;
            iterator++;
          }
        }
        if (line == "<MomentumBias>") {
          iterator = 0;
          while (iterator != layers[layerNumber].momentumBias.size()) {

            getline(file, line);
            std::istringstream(line) >> value;
            layers[layerNumber].momentumBias[iterator] = value;
            iterator++;
          }
        }
        if (line == "<Weights>") {
          tmpVector.conservativeResize(layers[layerNumber].weights.cols());
          iterator = 0;
          while (iterator != layers[layerNumber].weights.rows()) {
            getline(file, line);
            std::istringstream iss(line);
            iterator2 = 0;
            while(iterator2 != tmpVector.size()){
              iss >> value;
              tmpVector[iterator2] = value;
              iterator2++;
            }
            layers[layerNumber].weights.row(iterator) = tmpVector;
            iterator++;
          }
        }
        if (line == "<MomentumWeights>") {
          tmpVector.conservativeResize(layers[layerNumber].momentumWeights.cols());
          iterator = 0;
          while (iterator != layers[layerNumber].momentumWeights.rows()) {
            getline(file, line);
            std::istringstream iss(line);
            iterator2 = 0;
            while(iterator2 != tmpVector.size()){
              iss >> value;
              tmpVector[iterator2] = value;
              iterator2++;
            }
            layers[layerNumber].momentumWeights.row(iterator) = tmpVector;
            iterator++;
          }
        }
      }
    }
  }
}

void Net::train(int epoches, Dataset data){
  clog << "Starting training" << endl;
  auto start = std::chrono::system_clock::now();
  if(topology[0] != data.allBooks[0]->bookVector.size()){
    std::cerr << "Error: first layer in topology according to used dataset must be: " << data.allBooks[0]->bookVector.size() << '\n';
    return;
  }
  if(topology.back() != data.genres.size()){
    std::cerr << "Error: first layer in topology according to used dataset must be: " << data.genres.size() << '\n';
    return;
}


  int biggestGenre; //for macrosampling we detect the genre with most samples
  int max = 0;
  for (size_t g = 0; g < data.genres.size(); g++) {
    if(data.genres[g].books.size() > max){
      max = data.genres[g].books.size();
      biggestGenre = g;
    }
  }

  int genreBatch = batchesSize/data.genres.size();
  if(genreBatch == 0) genreBatch = 1;
  std::vector<ColVector> batchInputs;
  std::vector<ColVector> batchResults;

std::vector<int> genreIterators(data.genres.size(), 0); //iterators will tell the position in book vector for each genre
int booksTrained = 0;
for (size_t e = 0; e < epoches;) {
  for (size_t i = 0; i < genreBatch; i++) {
    for (size_t g = 0; g < data.genres.size(); g++) { //for every book from thr biggest genre we add to the batch one book from other genres
      std::vector<double> currentBookVector = data.genres[g].books[genreIterators[g]]->bookVector;
      std::vector<int> currentResultVector = data.genres[g].books[genreIterators[g]]->genre;
      ColVector tmpInput;
      ColVector tmpResult;
      int brokenInput = 0;
      tmpInput.conservativeResize(currentBookVector.size());
      tmpResult.conservativeResize(currentResultVector.size());
      for (size_t d = 0; d < currentBookVector.size(); d++) {//copying bookVector
        tmpInput[d] = currentBookVector[d];
        if(isnan(currentBookVector[d])){ //detecting corrupted input
          brokenInput = 1;
        }
      }
      if(brokenInput == 1){
        brokenInput = 0;
        genreIterators[g]++;
        g--;
        continue;
      }
      for (size_t d = 0; d < currentResultVector.size(); d++) {
        tmpResult[d] = (double)currentResultVector[d];
      }

      batchInputs.push_back(tmpInput);
      batchResults.push_back(tmpResult);
      genreIterators[g]++;
      if(genreIterators[g] == data.genres[g].books.size()) {
        if(g == biggestGenre) { //if we came to the end of biggest genre books vector, it is the epoch's end
          i = genreBatch;
          auto end = std::chrono::system_clock::now();
          clog << endl << "End of epoch: " << e+1 << endl;
          clog << "Error: " << error/(float)(booksTrained) << endl;
            chrono::duration<double> elapsed_seconds = end - start;
            clog << "computing time: " << elapsed_seconds.count() << endl;
          error = 0.0;
          booksTrained = 0;
          logNetwork();
          e++;
          start = std::chrono::system_clock::now();
        }
        genreIterators[g] = 0;
      }
    }
  }

for (int i = 0; i < batchInputs.size(); i++) {
  feedForward(batchInputs[i]);
  backPropagate(batchResults[i]);
  booksTrained++;
  //clog << "\rBooks trained: " << booksTrained;
}
if(batchInputs.size()!= 0) {
  updateWeights(batchInputs.size());
}

batchInputs.resize(0);
batchResults.resize(0);

}

}

vector<double> Net::getResult(vector<double> input){//function receives Input as std vector and returns Result as std vector
  if(topology[0] != input.size()){
    std::cerr << "Error: first layer in topology according to used dataset must be: " << input.size() << '\n';
  }
ColVector inputVector(input.size()); //covert std vector to Eigen vector
  for (size_t i = 0; i < input.size(); i++) {
    inputVector[i] = input[i];
  }
  feedForward(inputVector); //feed forward input
  vector<double> output = {};
  for (size_t i = 0; i < layers.back().output.size(); i++) {
    output.push_back(layers.back().output[i]); // get output
  }
  return output;
}
