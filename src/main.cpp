#include<iostream>
#include "net.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

bool fileExists(const string &name){
  ifstream f(name);
  return f.good();

}

void updateModelList(){
  string name;
  ifstream file("model/listofmodels.txt");
  std::vector<string> list = {};
  while (getline(file, name)) {
    if(fileExists("model/"+name)) list.push_back(name);
  }
  ofstream outfile("model/listofmodels.txt", std::ofstream::trunc);
  for (size_t i = 0; i < list.size(); i++) {
    outfile << list[i] << endl;
  }
}

int main(int argc, char const *argv[]) {

  string flag_in;

      if (argc > 1)
          flag_in = string(argv[1]);
      else
      {
          cerr << "Missing argument. Terminating." << endl;
          return 1;
      }
      if ((!flag_in.compare("--newDataset")))
      {
          if (argc > 2){
            flag_in = string(argv[2]);
            if ((!flag_in.compare("-train"))){
              Dataset dtrain("train");
              dtrain.createDataset("dataset/datasetsrc/traindatasrc.txt", "dataset/traindataset.txt");
              dtrain.fullInfo();
            }
            else if ((!flag_in.compare("-test"))){
              Dataset dtest("test");
              dtest.createDataset("dataset/datasetsrc/testdatasrc.txt", "dataset/testdataset.txt");
              dtest.fullInfo();
            }
          }
          else{
            Dataset dtrain("train");
            dtrain.createDataset("dataset/datasetsrc/traindatasrc.txt", "dataset/traindataset.txt");
            dtrain.fullInfo();
            Dataset dtest("test");
            dtest.createDataset("dataset/datasetsrc/testdatasrc.txt", "dataset/testdataset.txt");
            dtest.fullInfo();
          }
}

else if ((!flag_in.compare("--trainModel")))
{
  if (argc > 2){
    flag_in = string(argv[2]);
    if ((!flag_in.compare("-new"))){
      Dataset dtrain("train");
      dtrain.readDataset("dataset/traindataset.txt", "full");
      string modelName;
      double learningRate;
      double momentum;
      int batchSize;
      string modelPath;
      vector<int> topology = {};
      int epoches;
      updateModelList();
      while (true) {
      cout << endl << "Type in new model's name: " << endl;
      cin >> modelName;

      modelPath = "model/" + modelName;
      if(fileExists(modelPath)){
        clog << "** Error: Model already exists, create model with new name" << endl;
      }
      else{
        ofstream fout("model/listofmodels.txt", ios::app);
        fout << modelName << endl;
        break;
      }
    }
          string input;
    getline(cin, input);
    while (true) {
      int digit;
    cout << endl << "Type in new model's topology: [Example: 100 20 10 7]: " << endl;
    clog << "*Warning: First layer must be the same size as the number of elements your training dataset's vectors use (" << dtrain.allBooks[0]->bookVector.size() << ")" << endl;
    clog << "*Warning: Last layer must have the same size as number of genres in your train dataset (" << dtrain.genres.size() << ")" << endl;
    getline(cin, input);
    std::istringstream iss(input);
    while(iss >> digit){
      topology.push_back(digit);
      if(digit < 1) topology[0] = 0;
      }
      if(topology[0] != dtrain.allBooks[0]->bookVector.size() || topology.back() != dtrain.genres.size()){
        clog << "*** Wrong input!: Please type in correct numbers" << endl;
        topology.resize(0);
      }
      else{
        break;
      }
    }
    while(true){
    cout << endl << "Type in training's learning rate: [A number between 0.0 and 1.0]" << endl;
    cin >> learningRate;
    if(learningRate <= 0.0 || learningRate > 1.0){
      clog << "*** Wrong input!: Please type in a number between 0.0 and 1.0" << endl;
    }
    else{
      break;
      }
    }
  while(true){
  cout << endl << "Type in training's momentum: [A number between 0.0 and 1.0]" << endl;
  cin >> momentum;
  if(momentum <= 0.0 || momentum > 1.0){
    clog << "*** Wrong input!: Please type in a number between 0.0 and 1.0" << endl;
      }
  else{
    break;
      }
    }
    while(true){
      cout << endl << "Type in size of batches to use while training: [A number bigger than 0]" << endl;
      cin >> batchSize;
      if(batchSize <= 0){
        clog << "*** Wrong input!: Please type in a number bigger than 0" << endl;
      }
      else{
        break;
      }
    }
    while(true){
      cout << endl << "Type in number of epoches for training: [A number bigger than 0]" << endl;
      cin >> epoches;
      if(epoches <= 0){
        clog << "*** Wrong input!: Please type in a number bigger than 0" << endl;
      }
      else{
        break;
      }
    }
    Net net(modelPath, topology, learningRate, momentum, batchSize);
    net.train(epoches, dtrain);
    }
  }
  else{
    string modelName;
    double learningRate;
    double momentum;
    int batchSize;
    int epoches;
    updateModelList();
    cout << "-------------------------" << endl;
    cout << "List of available models:" << endl;
    ifstream file("model/listofmodels.txt");
    string name;
    while (getline(file, name)) {
      cout << name << endl;
    }
    cout << "-------------------------" << endl << endl;
    bool correctInput = false;
    while(correctInput == false){
      cout << "Please type in model's name you want to use: [example: model1.txt]" << endl;
      cin >> modelName;
      ifstream file("model/listofmodels.txt");
      while (getline(file, name)) {
        if(name == modelName) {
          correctInput = true;
          break;
          }
        }
        if(correctInput == false) cout << "*** Wrong input!: please type in the name of an existing model, or create a new one by using '-new' argument" << endl;
      }
      while(true){
        cout << endl << "Type in training's learning rate: [A number between 0.0 and 1.0]" << endl;
        cin >> learningRate;
          if(learningRate <= 0.0 || learningRate > 1.0){
            clog << "*** Wrong input!: Please type in a number between 0.0 and 1.0" << endl;
          }
          else{
            break;
          }
        }
      while(true){
        cout << endl << "Type in training's momentum: [A number between 0.0 and 1.0]" << endl;
        cin >> momentum;
        if(momentum <= 0.0 || momentum > 1.0){
          clog << "*** Wrong input!: Please type in a number between 0.0 and 1.0" << endl;
        }
        else{
          break;
        }
      }
      while(true){
        cout << endl << "Type in size of batches to use while training: [A number bigger than 0]" << endl;
        cin >> batchSize;
        if(batchSize <= 0){
          clog << "*** Wrong input!: Please type in a number bigger than 0" << endl;
        }
        else{
          break;
        }
      }
      while(true){
        cout << endl << "Type in number of epoches for training: [A number bigger than 0]" << endl;
        cin >> epoches;
        if(epoches <= 0){
          clog << "*** Wrong input!: Please type in a number bigger than 0" << endl;
        }
        else{
          cout << endl;
          break;
        }
      }
      Dataset dtrain("train");
      dtrain.readDataset("dataset/traindataset.txt", "full");
      Net net("model/"+modelName, learningRate, momentum, batchSize);
      net.readNetwork();
      net.train(epoches, dtrain);
    }
  }
  else if ((!flag_in.compare("--test"))){
    if (argc > 2){
      flag_in = string(argv[2]);
    }
      if ((!flag_in.compare("-network")) || argc == 2){
        string modelName;
        updateModelList();
        cout << "Network test:" << endl << endl;
        cout << "-------------------------" << endl;
        cout << "List of available models:" << endl;
        ifstream file("model/listofmodels.txt");
        string name;
        while (getline(file, name)) {
          cout << name << endl;
        }
        cout << "-------------------------" << endl << endl;
        bool correctInput = false;
        while(correctInput == false){
          cout << "Please type in model's name you want to use: [example: model100_7.txt]" << endl;
          cin >> modelName;
          ifstream file("model/listofmodels.txt");
          while (getline(file, name)) {
            if(name == modelName) {
              correctInput = true;
              break;
              }
            }
            if(correctInput == false) cout << "*** Wrong input!: please type in the name of an existing model, or create a new one by using '-new' argument" << endl;
          }
          Dataset dtest("test");
          dtest.readDataset("dataset/testdataset.txt", "full");
          dtest.fullInfo();
          dtest.testDatasetNetwork("model/"+modelName);
      }
      if ((!flag_in.compare("-bayes")) || argc == 2){
        cout << "Bayes test:" << endl << endl;
        Dataset dtrain("train");
        dtrain.readDataset("dataset/traindataset.txt", "partial");
        Dataset dtest("test");
        dtest.readDataset("dataset/testdataset.txt", "full");
        dtest.testDatasetBayes(dtrain);
      }
      if ((!flag_in.compare("-single"))){
        cout << "Single test:" << endl << endl;
        clog << "*Warning: this operation will need to load full dictionary into RAM (3.5GB), please clean memory before testing" << endl << endl;
        Dataset dtrain("train");
        dtrain.readDataset("dataset/traindataset.txt", "partial");
        Dataset dtest("test");
        updateModelList();
        cout << "-------------------------" << endl;
        cout << "List of available models:" << endl;
        ifstream file("model/listofmodels.txt");
        string name;
        string modelName;
        while (getline(file, name)) {
          cout << name << endl;
        }
        cout << "-------------------------" << endl << endl;
        bool correctInput = false;
        while(correctInput == false){
          cout << "Please type in model's name you want to use: [example: model1.txt]" << endl;
          cin >> modelName;
          ifstream file("model/listofmodels.txt");
          while (getline(file, name)) {
            if(name == modelName) {
              correctInput = true;
              cout << endl;
              break;
            }
          }
            if(correctInput == false) cout << "*** Wrong input!: please type in the name of an existing model" << endl;
        }
        string text;
        getline(cin, text);
        while (true) {
          cout << "Type in book description: [for exit type EXIT; for changing network model type CHANGE MODEL]" << endl;
          getline(cin, text);
          cout << endl;
          if(text == "EXIT") break;

          if(text == "CHANGE MODEL"){
            cout << "-------------------------" << endl;
            cout << "List of available models:" << endl;
            ifstream file("model/listofmodels.txt");
            while (getline(file, name)) {
              cout << name << endl;
            }
            cout << "-------------------------" << endl << endl;
            bool correctInput = false;
            while(correctInput == false){

              //getline(cin, modelName);
              cout << "Please type in model's name you want to use: [example: model1.txt]" << endl;
              cin >> modelName;
              ifstream file("model/listofmodels.txt");
              while (getline(file, name)) {
                if(name == modelName) {
                  correctInput = true;
                  cout << endl;
                  break;
                }
              }
                if(correctInput == false) cout << "*** Wrong input!: please type in the name of an existing model" << endl;
            }
            getline(cin, text);
            continue;
          }
            dtest.testSingleBook(text, dtrain, modelName);
        }
      }
  }
  return 0;
}
