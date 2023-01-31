
#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>
#include <future>
#include <map>
#include "eigen3/Eigen/Eigen"



using namespace std;


struct Word{
  string text;
  int repeatings;

  Word(){repeatings = 1;};
  Word(string t, int r){
    text = t;
    repeatings = r;
  }
  Word(const Word &w){text = w.text; repeatings = w.repeatings;} //copy constructor
};

struct Book {

  int id;
  int length;

  vector<int> genre = {};
  std::map<std::string, std::vector<double>> computedGenres;
  vector<Word> pWordsInBook;
  vector<Word> wordsInBook;
  vector<double> bookVector;

  Book(int idNumber){
    length = 0;
    id = idNumber;
  }
  void fillWords(const string &text);
  void writeBook(string fname);
};

struct Genre {
  string genreName;
  int booksCount;
  int totalWordCount;

  std::vector<int> genreVec = {};
  vector<Word> pUniqueWords;
  vector<Word> uniqueWords;//j
  vector<shared_ptr<Book>> books;


  int truePositive = 0;
  int falseNegative = 0;
  int falsePositive = 0;


  Genre(string name, vector<int> vec){
    booksCount = 0;
    totalWordCount = 0;
    genreName = name;
    genreVec = vec;
  }
  void fullInfo(string mode);
  void fillWords(void);

};

struct Dataset{
  string type; //type = ("test"/"train")
  int booksCount;
  int totalWordCount;

  std::vector<Word> neutralWords;
  std::vector<Genre> genres;
  std::vector<shared_ptr<Book>> allBooks;
  std::map<string, vector<double>> dictionary;

  void testDatasetBayes(const Dataset &trainDataset);
  void testDatasetNetwork(const string &modelname);
  void testBookBayes(shared_ptr<Book> testbook, const Dataset &trainDataset, int * threadState);
  void testBookNetwork(shared_ptr<Book> testbook, const string &modelName);

  int truePositive = 0;
  int falseNegative = 0;
  int falsePositive = 0;


  Dataset(string setType){
    totalWordCount = 0;
    booksCount = 0;
    type = setType;
    booksCount = 0;
  }

  void testSingleBook(string &text, const Dataset &trainDataset, const string &modelName);
  void printDataset(const string &outfile); //mode = (full/partial)
  void createDataset(string infile, string outfile);
  void fillNeutralWords(const int &vectorSize);
  void fillBookVectors(void);
  void readDataset(const string &fname, const string &mode);
  void loadDictionary(const string &fname);
  void loadStatistics(string mode);
  void fullInfo();
  void calculateComputedError(const vector<double> &output, const vector<int> &expectedResult, const double &decisionFactor);
};
