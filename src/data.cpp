
#include "net.hpp"
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <memory>
#include "OleanderStemmingLibrary-master/include/olestem/stemming/english_stem.h"
#include <cctype>
#include <algorithm>
#include <cstring>
#include <thread>
#include <iomanip>
#include <limits>
#include <iomanip>
#include <thread>
#include <chrono>
#include <ctime>
#include <future>
#include <map>

using namespace std;

bool compareByRepeatings(const Word &a, const Word &b){
    return a.repeatings > b.repeatings;
}

string stemmed(string ANSIWord){ //porter stemming using extern library
    wchar_t* UnicodeTextBuffer = new wchar_t[ANSIWord.length()+1];
    std::wmemset(UnicodeTextBuffer, 0, ANSIWord.length()+1);
    std::mbstowcs(UnicodeTextBuffer, ANSIWord.c_str(), ANSIWord.length());
    wstring word = UnicodeTextBuffer;
      stemming::english_stem<wstring> StemEnglish;
      StemEnglish(word);
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    delete[] UnicodeTextBuffer;
    return converter.to_bytes(word);
    return ANSIWord;
}

void Dataset::createDataset(string infile, string outfile){
clog << "Creating new " << type << " dataset" << endl;
  string line;
  string body;

if(type == "test"){
    ifstream trainDataset("dataset/traindataset.txt"); //open train dataset for genres info
    if(!trainDataset){
      std::cerr << "ERROR: cannot open train dataset file" << '\n';
      std::cerr << "ERROR: create train dataset before creating test dataset" << endl;
      exit(1);
    }
    while(getline(trainDataset, line)){
      if(line.find("<Dataset>") != string::npos){}
      if(line.find("<Genre>") != string::npos){
        string name;
        vector<int> genreVec = {};
        getline(trainDataset, name);
        getline(trainDataset, line);
        int number;
        std::istringstream iss(line);
        while (iss >> number) {
          genreVec.push_back(number);
        }
        Genre g(name, genreVec);
        genres.push_back(g); //creating same genres as in training dataset
      }
    if(line.find("<Book>") != string::npos) break;
}
}


ifstream file(infile);
if(!file){
  std::cerr << "ERROR: cannot open " << type << " dataset source file" << '\n';
  exit(1);
}
else{
  std::clog << "opened " << type << " dataset source file" << '\n';
  clog << "begin to load data" << endl;

}


while (getline(file, line)) {
  if(line.find("<body>") != string::npos){ //found new book
    booksCount++;
    clog <<"\rBooks loaded: " << booksCount;
    body = line.substr(6, line.length()-13); //getting rid of <body>

    do {getline(file, line);}
    while(line.find("<topics>") == string::npos); //found topics
    getline(file, line);

    size_t foundB = -1; //cursor in topic line
    size_t foundE = -1;
    do{ //finding all book genres
      foundB++;
      foundE++;
       foundB = line.find("<d0>", foundB);
       foundE = line.find("</d0>", foundE);
       string genre = line.substr(foundB+4, foundE-4-foundB); //extracting genre name
       int createNewGenre = 1;
       for(int i = 0; i < genres.size(); i++){ //finding if genre exists in database
         if(genres[i].genreName == genre){ //if genre exists, create new book in that genre

           shared_ptr<Book> b(new Book(booksCount+1000000));
           b->genre = genres[i].genreVec;
           //check for duplicates in allBooks
           if(allBooks.size()>0 && allBooks.back()->id == b->id) {//if book exists, add it to new genre
             for (size_t n = 0; n < allBooks.back()->genre.size(); n++) {
              allBooks.back()->genre[n] = b->genre[n] + allBooks.back()->genre[n];
             }
                  genres[i].books.push_back(allBooks.back());
           }
           else{ //else create new book
             b->fillWords(body);
             genres[i].books.push_back(b);
             totalWordCount += b->length;
             allBooks.push_back(b);
             genres[i].booksCount++;
         }
         createNewGenre = 0;
         }
       }
       if(createNewGenre == 1){ //if genre does not exist, create new genre and new book in that genre
         std::vector<int> newGenreVector = {};
         for (size_t i = 0; i < genres.size(); i++) { //create genre vector
           newGenreVector.push_back(0);
           genres[i].genreVec.push_back(0); //resize other genre Vectors
         }
         for (size_t l = 0; l < allBooks.size(); l++) { //resize other book's genre Vectors
           allBooks[l]->genre.push_back(0);
         }
         newGenreVector.push_back(1);
         Genre g(genre, newGenreVector);
         shared_ptr<Book> b(new Book(booksCount+1000000));
         b->genre = g.genreVec;
         g.booksCount = 1;
//check for duplicates in allBooks
           if(allBooks.size()>0 && allBooks.back()->id == b->id) { //if book exists, add it to new genre
             for (size_t n = 0; n < allBooks.back()->genre.size(); n++) {
               allBooks.back()->genre[n] = b->genre[n] + allBooks.back()->genre[n];
             }
             g.books.push_back(allBooks.back());
           }
         else{ //else create new book
           b->fillWords(body);
            g.books.push_back(b);
            totalWordCount += b->length;
           allBooks.push_back(b);
       }
       genres.push_back(g);
       }
    }
    while(line.find("<d0>", foundB+1) != string::npos);
  }
  else{
    continue;
  }
}

clog << endl << "loading genres data" << endl;
for (size_t i = 0; i < genres.size(); i++) {
  genres[i].fillWords();
}
clog << "finding neutral words" << endl;
fillNeutralWords(200);
clog << endl;
fillBookVectors();
clog << "saving dataset" << endl;
printDataset(outfile);
}

void Dataset::fillNeutralWords(const int &vectorSize){ //finds most repeated words and fill them into neutralWords vector
  std::vector<Word> vtmp = {};
  for (size_t g = 0; g < genres.size(); g++) {
  for (size_t w = 0; w < genres[g].uniqueWords.size(); w++) {
    int newWord = 1;
    for (size_t tw = 0; tw < vtmp.size(); tw++) {
      if(genres[g].uniqueWords[w].text == vtmp[tw].text){
        vtmp[tw].repeatings += genres[g].uniqueWords[w].repeatings;
        newWord = 0;
        break;
      }
    }
    if(newWord == 1){
      vtmp.push_back(genres[g].uniqueWords[w]);
    }
  }
}
sort(vtmp.begin(), vtmp.end(), compareByRepeatings);
vtmp.resize(vectorSize);
neutralWords = vtmp;
}

void Book::fillWords(const string &text){ //fills pWordsInBook and wordsInBook
  string currentWord = "";
  for(auto x:text){
if(!isalpha(x)) x = ' ';
if(x == ' '){
  std::transform(currentWord.begin(), currentWord.end(), currentWord.begin(),[](unsigned char c){return std::tolower(c);});
  if(currentWord[0] == ' ' || currentWord.length() == 0) {
    currentWord = "";
    continue;
  }
  length++;
  int newPWord = 1;
  int newWord = 1;
  for (size_t k = 0; k < pWordsInBook.size(); k++) {
    if(pWordsInBook[k].text == stemmed(currentWord)){
      pWordsInBook[k].repeatings++;
            newPWord = 0;
      break;
    }
  }
  for (size_t i = 0; i < wordsInBook.size(); i++) {
    if(wordsInBook[i].text == currentWord){
      wordsInBook[i].repeatings++;
      newWord = 0;
      break;
    }
  }
  if(newPWord == 1){
      Word pw(stemmed(currentWord), 1);
      pWordsInBook.push_back(pw);
  }
  if(newWord == 1){
      Word w(currentWord, 1);
      wordsInBook.push_back(w);
  }
  currentWord = "";
}
else{
  currentWord+=x;
}
}

}

void Genre::fillWords(void){
  for (size_t i = 0; i < books.size(); i++) { //going through all books
    for (size_t w = 0; w < books[i]->pWordsInBook.size(); w++) { //and every word in each book
      size_t gw;
      for (gw = 0; gw < pUniqueWords.size(); gw++) {
        if(books[i]->pWordsInBook[w].text == pUniqueWords[gw].text){ //if the word is already in pUniqueWords, add it's repeatings
          pUniqueWords[gw].repeatings+=books[i]->pWordsInBook[w].repeatings;
          break;
        }
      }
if(gw == pUniqueWords.size()){ //if the word is not in pUniqueWords, add it
  Word g(books[i]->pWordsInBook[w].text, books[i]->pWordsInBook[w].repeatings);
  pUniqueWords.push_back(g);
}
    }

    for (size_t w = 0; w < books[i]->wordsInBook.size(); w++) { //same but for not stemmed words
      size_t gw;
      for (gw = 0; gw < uniqueWords.size(); gw++) {
        if(books[i]->wordsInBook[w].text == uniqueWords[gw].text){
          uniqueWords[gw].repeatings+=books[i]->wordsInBook[w].repeatings;
          totalWordCount += books[i]->wordsInBook[w].repeatings; //counting every word for totalWordCount
          break;
        }
      }
if(gw == uniqueWords.size()){
  Word g(books[i]->wordsInBook[w].text, books[i]->wordsInBook[w].repeatings);
  totalWordCount += books[i]->wordsInBook[w].repeatings; //counting every word
  uniqueWords.push_back(g);
}
    }
  }

}

void Dataset::printDataset(const string &outfile){
  clog << "Printing " << type << " dataset" << endl;
  ofstream outdata(outfile, std::ofstream::trunc);
  if(!outdata) cerr << "Error: file for writing created dataset could not be opened" << endl;
  outdata << "<Dataset>" << endl;
  outdata << "<MainInfo>" << endl;
  outdata << type << endl;
  outdata << booksCount << endl;
  outdata << totalWordCount << endl;
  for (size_t i = 0; i < neutralWords.size(); i++) {
    outdata << neutralWords[i].text << " " << neutralWords[i].repeatings << " ";
  }
  outdata << endl;
  outdata << "</MainInfo>" << endl;
  for (size_t g = 0; g < genres.size(); g++) {
    outdata << "<Genre>" << endl;
    outdata << genres[g].genreName << endl;
    for (size_t i = 0; i < genres[g].genreVec.size(); i++) {
      outdata << genres[g].genreVec[i] << " ";
    }
    outdata << endl;
    outdata << genres[g].booksCount << endl;
    outdata << genres[g].totalWordCount << endl;
    typedef std::numeric_limits<double> dbl;
    outdata.precision(dbl::max_digits10);
    for (size_t w = 0; w < genres[g].pUniqueWords.size(); w++) {
      outdata << genres[g].pUniqueWords[w].text << " " << genres[g].pUniqueWords[w].repeatings << " ";
    }
    outdata << endl;
    for (size_t w = 0; w < genres[g].uniqueWords.size(); w++) {
      outdata << genres[g].uniqueWords[w].text << " " << genres[g].uniqueWords[w].repeatings << " ";
    }
    outdata << endl;

    outdata << "</Genre>" << endl;
  }
  for (size_t b  = 0; b  < allBooks.size(); b ++) {
    outdata << "<Book>" << endl;
    outdata << allBooks[b]->id << endl;
    outdata << allBooks[b]->length << endl;
    for (size_t i = 0; i < allBooks[b]->genre.size(); i++) {
      outdata << allBooks[b]->genre[i] << " ";
    }
    outdata << endl;
    for (size_t i = 0; i < allBooks[b]->pWordsInBook.size(); i++) {
      outdata << allBooks[b]->pWordsInBook[i].text << " " << allBooks[b]->pWordsInBook[i].repeatings << " ";
    }
    outdata << endl;
    for (size_t i = 0; i < allBooks[b]->wordsInBook.size(); i++) {
      outdata << allBooks[b]->wordsInBook[i].text << " " << allBooks[b]->wordsInBook[i].repeatings << " ";
    }
    outdata << endl;
    typedef std::numeric_limits<double> dbl;
    outdata.precision(dbl::max_digits10);
    for (size_t i = 0; i < allBooks[b]->bookVector.size(); i++) {
      outdata << allBooks[b]->bookVector[i] << " ";
    }
    outdata << endl;
    outdata << "</Book>" << endl;
  }
  outdata << "</Dataset>" << endl;
}

void Dataset::readDataset(const string &fname, const string &mode){
  clog << "Reading " << type << " dataset" << endl;
  ifstream file(fname);
  if(!file){
    std::cerr << "Error: cannot open dataset file" << '\n';
  }
  string line;
  while(getline(file, line)){
    if(line.find("<Dataset>") != string::npos) clog << "Data found, begin to load " << type << " dataset" << endl;
    if(line.find("<MainInfo>") != string::npos) {
    //  clog << "Loading main info" << endl;
      getline(file, line);
      std::istringstream(line) >> type;
      getline(file, line);
      std::istringstream(line) >> booksCount;
      getline(file, line);
      std::istringstream(line) >> totalWordCount;
      getline(file, line);
      std::istringstream iss(line);
      string word;
      int repeating;
    if(mode == "full" || (mode == "partial" && type == "train") ){
      while (iss >> word) {
        iss >> repeating;
        Word w(word, repeating);
        neutralWords.push_back(w);
      }
    }
    }
    if(line.find("<Genre>") != string::npos) {
      string name;
      std::vector<int> genreVec;
      int number;
      getline(file, line);
      name = line;
      getline(file, line);
      std::istringstream iss(line);
      while (iss >> number) {
        genreVec.push_back(number);
      }
      Genre g(name, genreVec);
      genres.push_back(g);
      getline(file, line);
      std::istringstream(line) >> genres.back().booksCount;
      getline(file, line);
      std::istringstream(line) >> genres.back().totalWordCount;
      string word;
      int repeating;

      getline(file, line);
      std::istringstream iss2(line);
      if(mode == "full" || (mode == "partial" && type == "train") ){
      while (iss2 >> word) {
        iss2 >> repeating;
        Word w(word, repeating);
        genres.back().pUniqueWords.push_back(w);
      }
      getline(file, line);
      std::istringstream iss3(line);
      while (iss3 >> word) {
        iss3 >> repeating;
        Word w(word, repeating);
        genres.back().uniqueWords.push_back(w);
      }
    }
    }

    if(line.find("<Book>") != string::npos){
      if(mode == "partial" && type == "train"){
        break;
      }
      int id;
      getline(file, line);
      std::istringstream(line) >> id;
      shared_ptr<Book> b(new Book(id));
      getline(file, line);
      std::istringstream(line) >> b->length;
      getline(file, line);
      int number;
      std::istringstream iss(line);
      while (iss >> number) {
        b->genre.push_back(number);
      }
      string word;
      int repeating;
      getline(file, line);
      std::istringstream iss2(line);
      while (iss2 >> word) {
        iss2 >> repeating;
        Word w(word, repeating);
        b->pWordsInBook.push_back(w);
      }
      getline(file, line);
      std::istringstream iss3(line);
      while (iss3 >> word) {
        iss3 >> repeating;
        Word w(word, repeating);
        b->wordsInBook.push_back(w);
      }
      getline(file, line);
      std::istringstream iss4(line);
      double digit;
      while (iss4 >> digit) {
        b->bookVector.push_back(digit);
      }
      allBooks.push_back(b);
      for (size_t i = 0; i < b->genre.size(); i++) {
        if(b->genre[i] == 1) genres[i].books.push_back(b);
      }
    }
    if(mode == "full") clog << "\rBooks loaded: " << allBooks.size() << "/" << booksCount;
  }
  clog << endl;
}

void Dataset::testBookBayes(shared_ptr<Book> testbook, const Dataset &trainDataset, int * threadState){
  vector<double> computedGenre;
  double smoothing = 0.001;
  int textLength = testbook->length;
  computedGenre.resize(0);
  vector<double> oneProbability = {};
  vector<double> restProbability = {};
  vector<int> repeatings;
  for (size_t g = 0; g < trainDataset.genres.size(); g++) { //init computed genre vector
    computedGenre.push_back(0.0);
    oneProbability.push_back(0);
    restProbability.push_back(0);
  }
  for (size_t w = 0; w < testbook->pWordsInBook.size(); w++) {
    int neutral = 0;
    for (size_t n = 0; n < trainDataset.neutralWords.size(); n++) { //checking for neutral words
      if (testbook->pWordsInBook[w].text == trainDataset.neutralWords[n].text) {
        neutral = 1;
        break;
      }
    }
    if(neutral == 1) continue;
    repeatings.resize(trainDataset.genres.size());
    fill(repeatings.begin(), repeatings.end(), 0);
for (size_t g = 0; g < trainDataset.genres.size(); g++) { //going throgh every genre to calculate probability of word being in that genre
  Genre currentGenre = trainDataset.genres[g];
  size_t w2;
  for (w2 = 0; w2 < currentGenre.pUniqueWords.size(); w2++) { //going through every word in genre to find the one we are looking for

    if(currentGenre.pUniqueWords[w2].text == testbook->pWordsInBook[w].text && oneProbability[g] == 0) {//if we found the word and oneProbabiloty is still 0
      //add the first value of oneProbability
      oneProbability[g] = (double)testbook->pWordsInBook[w].repeatings * ((double)currentGenre.pUniqueWords[w2].repeatings + smoothing)/(double)(currentGenre.totalWordCount + smoothing*textLength);
      repeatings[g] = currentGenre.pUniqueWords[w2].repeatings;
      break;
    }
    else if(currentGenre.pUniqueWords[w2].text == testbook->pWordsInBook[w].text){//if we found the word
      //new value of oneProbability
      oneProbability[g] *= ((double)testbook->pWordsInBook[w].repeatings * (((double)currentGenre.pUniqueWords[w2].repeatings + smoothing)/(double)(currentGenre.totalWordCount + smoothing*textLength)));
      repeatings[g] = currentGenre.pUniqueWords[w2].repeatings;
      break;
  }

}
if(w2 == currentGenre.pUniqueWords.size() && oneProbability[g] == 0){//if word not found
   oneProbability[g] = (double)testbook->pWordsInBook[w].repeatings * smoothing/(double)(currentGenre.totalWordCount + smoothing*textLength);
   repeatings[g] = 0;
   continue;
}
else if(w2 == currentGenre.pUniqueWords.size()){
   oneProbability[g] *= ((double)testbook->pWordsInBook[w].repeatings * smoothing/(double)(currentGenre.totalWordCount + smoothing*textLength));
   repeatings[g] = 0;
}
  }
  //calculate restProbability
  for (size_t g = 0; g < trainDataset.genres.size(); g++) {
    int restRepeatings = 0;
    int restWordCount = trainDataset.totalWordCount - trainDataset.genres[g].totalWordCount;
  for (size_t g2 = 0; g2 < trainDataset.genres.size(); g2++) {
  if(g2 == g) continue;
  restRepeatings += repeatings[g2];
}
  if(restRepeatings == 0){
  if(restProbability[g] == 0){
    restProbability[g] = (double)testbook->pWordsInBook[w].repeatings * smoothing/(double)restWordCount + smoothing*textLength;
  }
  else{
    restProbability[g] *= ((double)testbook->pWordsInBook[w].repeatings * smoothing/(double)restWordCount + smoothing*textLength);
  }
  }
  else{
  if(restProbability[g] == 0){
    restProbability[g] = (double)testbook->pWordsInBook[w].repeatings * ((double)restRepeatings + smoothing)/(double)(restWordCount + smoothing*textLength);
  }
  else{
    restProbability[g] *= ((double)testbook->pWordsInBook[w].repeatings * ((double)restRepeatings + smoothing)/(double)(restWordCount + smoothing*textLength));
  }
  }
  }
}
  for (size_t d = 0; d < computedGenre.size(); d++) {
      oneProbability[d] = oneProbability[d] * (double)trainDataset.genres[d].booksCount/(double)trainDataset.booksCount;
      restProbability[d] = restProbability[d] * (1.0 - ((double)trainDataset.genres[d].booksCount/(double)trainDataset.booksCount));
    computedGenre[d] = oneProbability[d]/restProbability[d];
  }
    if(allBooks.size() > 1) calculateComputedError(computedGenre, testbook->genre, 1.0);
    if(testbook->computedGenres.find("Bayes") == testbook->computedGenres.end()){
      testbook->computedGenres.insert({"Bayes", computedGenre});
    }
    else{
      auto itr = testbook->computedGenres.find("Bayes");
      itr->second = computedGenre;
    }
  *threadState = 0;
}

void Dataset::testDatasetBayes(const Dataset &trainDataset){

  truePositive = 0;
  falseNegative = 0;
  falsePositive = 0;

  for (size_t g = 0; g < genres.size(); g++) {
    genres[g].truePositive = 0;
    genres[g].falseNegative = 0;
    genres[g].falsePositive = 0;
  }

    int NUM_THREADS = 5;
      int booksToTest = allBooks.size();
      while (true) {
      cout << "Type in number of books you want to test: " << endl;
      cout << "[If you want to test whole dataset, type in -1]" << endl;
      cin >> booksToTest;
      cout << endl;
      if(booksToTest == -1) booksToTest = booksCount;
      if(booksToTest < 1) clog << "*** Wrong input!: number must be bigger than 0" << endl;
      else if(booksToTest > booksCount) clog << "Wrong input!: number is bigger than number of books available" << endl;
      else break;
      clog << endl;
    }

      while(true){
    cout << "Type in number of threads to use while testing [recommended: 20]" << endl;;
    cin >> NUM_THREADS;
    cout << endl;
    if(NUM_THREADS < 1) clog << "*** Wrong input: number of threads must be bigger than 0" << endl;
    else if(NUM_THREADS > booksToTest) clog << "*** Wrong input!: number of threads cannot be bigger than number of books to test" << endl;
    else break;
      cout << endl;
    }

      cout << "Begin to test dataset" << endl;
      vector<int> threadActivity;
      threadActivity.resize(0);
      for (int i = 0; i < NUM_THREADS; i++) {
          threadActivity.push_back(0);
      }
      if(booksToTest < 1) booksToTest = allBooks.size();
    auto start = std::chrono::system_clock::now();
    vector<thread> threadVector;

    clog << endl;

      int b = 0;
      for(int t = 0; t < NUM_THREADS; t++){
          int * tPointer = &threadActivity.at(t);
          thread th(&Dataset::testBookBayes, this, allBooks[b], trainDataset, tPointer);
          threadVector.push_back(move(th));
          threadActivity[t] = 1;
          b++;
      }
      clog << "\rBooks tested " << 0;


      while(b < booksToTest /*allBooks.size()*/){

          for (int t = 0; t < NUM_THREADS && b < booksToTest; t++) {
              if(threadActivity[t] == 0){
                  threadVector[t].join();
                  clog << "\rBooks tested " << b+1;
                  int * tPointer = &threadActivity.at(t);
                  thread th(&Dataset::testBookBayes, this, allBooks[b], trainDataset, tPointer);
                  threadVector[t] = move(th);
                  threadActivity[t] = 1;
                  b++;
              }
          }
      }
  for (int t = 0; t < NUM_THREADS; t++) {
        threadVector[t].join();
    }
    clog << endl;
    auto end = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    clog << "computing time: " << elapsed_seconds.count() << endl;
    clog << endl;
    clog << "==========================" << endl;
    clog << "RESULTS BAYES" << endl;
    clog << endl;
    clog << "BOOKS TESTED:    " << booksCount << endl;
    clog << "TRUE POSITIVE:   " << truePositive << endl;
    clog << "FALSE NEGATIVE:  " << falseNegative << endl;
    clog << "FALSE POSITIVE:  " << falsePositive << endl;
    clog << endl;
    double precision = (double)truePositive/(double)(truePositive+falsePositive);
    double recall = (double)truePositive/(double)(truePositive + falseNegative);
    double f1 = (2*precision*recall)/(precision+recall);
    clog << "F1 = " <<f1 << endl;
    double macroAverage = 0.0;
    vector<double> genreScores = {};
    for (size_t g = 0; g < genres.size(); g++) {
      double genrePrecision = (double)genres[g].truePositive/(double)(genres[g].truePositive+genres[g].falsePositive);
      double genreRecall = (double)genres[g].truePositive/(double)(genres[g].truePositive + genres[g].falseNegative);
      double genreF1 = (2*genrePrecision*genreRecall)/(genrePrecision+genreRecall);
      genreScores.push_back(genreF1);
      macroAverage += genreF1;
    }
    macroAverage = macroAverage/(double)genres.size();
  clog << "AVG = " << macroAverage << endl;
    for (size_t g = 0; g < genres.size(); g++) {
      clog << endl << "GENRE " << genres[g].genreName << endl;
      clog << "F1 = " << genreScores[g] << endl;
      clog << "TRUE POSITIVE | FALSE NEGATIVE | FALSE POSITIVE" << endl;
      clog << setw(13) << genres[g].truePositive << " | ";
      clog << setw(14) << genres[g].falseNegative << " | ";
      clog << setw(14) << genres[g].falsePositive << endl;
    }

    clog << "==========================" << endl;
  }

void Dataset::fullInfo(){
    cout << endl;
    cout << "===================================" << endl;
    cout << "DATASET INFO " << endl;
    cout << "TYPE:                 "  << type << endl;
    cout << "BOOKS:                " << booksCount << endl;
    cout << "WORDS:                " << totalWordCount << endl;
    cout << "NEUTRAL WORDS:        " << neutralWords.size() << endl;
    cout << endl;
    cout << "GENRES: " << endl;
      for (size_t g = 0; g < genres.size(); g++) {
      cout << endl;
      cout << "-----------------------------------" << endl;
      cout << "NAME:                 " << genres[g].genreName << endl;
      cout << "BOOKS:                " << genres[g].booksCount << endl;
      cout << "WORDS:                " << genres[g].totalWordCount << endl;
      cout << "VECTOR:               ";
      for (size_t i = 0; i < genres[g].genreVec.size(); i++) {
        cout << genres[g].genreVec[i] << " ";
      }
      cout << endl;
      cout << "UNIQUE WORDS:         " << genres[g].uniqueWords.size() << endl;
      cout << "PORTER STEMMED WORDS: " << genres[g].pUniqueWords.size() << endl;
      cout << "-----------------------------------" << endl;
    }
    cout << "===================================" << endl;
   }

void Dataset::loadDictionary(const string &fname){
  ifstream file(fname);
  if(!file){
    std::cerr << "Error: cannot open dictionary file" << '\n';
  }
  else{
    std::clog << "Loading dictionary" << '\n';
  }
  string line;
  int numberOfWords;
  getline(file, line);
  std::istringstream(line) >> numberOfWords;
  while (getline(file, line)){
    string word;
    double number;
    vector<double> tmpVector = {};
    std::istringstream iss(line);
    iss >> word;
    while (iss >> number) {
      tmpVector.push_back(number);
    }
    dictionary.insert({word, tmpVector});
    clog << "\rWords loaded to dictionary: " << dictionary.size() << "/" << numberOfWords;
  }
  clog << endl;
}

void Dataset::fillBookVectors(void){
  if(dictionary.empty()) loadDictionary("dataset/datasetsrc/wordvectors.txt");
  if(allBooks.size()>1) clog << "Calculating bookVectors" << endl;
  for (size_t b = 0; b < allBooks.size(); b++) {
    allBooks[b]->bookVector.resize(dictionary.begin()->second.size());
    fill(allBooks[b]->bookVector.begin(), allBooks[b]->bookVector.end(), 0.0);

    for (size_t w = 0; w < allBooks[b]->wordsInBook.size(); w++) {
      Word currentWord = allBooks[b]->wordsInBook[w];
      //clog << "searching for " << currentWord.text << endl;
      int neutral = 0;
      for (size_t i = 0; i < neutralWords.size(); i++) {
        if(currentWord.text == neutralWords[i].text) neutral = 1;
      }
      if(neutral == 1){
        continue;
      }
      auto itr = dictionary.find(currentWord.text);
      if(itr == dictionary.end()){
        continue;
      }
      //clog << "found " << itr->first << endl;
      for (size_t i = 0; i < allBooks[b]->bookVector.size(); i++) {
        allBooks[b]->bookVector[i] += (itr->second[i] * currentWord.repeatings);
      }
    }

    double vectorSize = 0.0;
    for (size_t i = 0; i < allBooks[b]->bookVector.size(); i++) {
      vectorSize += abs(allBooks[b]->bookVector[i]);
    }
    for (size_t i = 0; i < allBooks[b]->bookVector.size(); i++) {
      allBooks[b]->bookVector[i] *= (1.0/vectorSize);
    }
      if(allBooks.size()>1)clog << "\rVectors calculated: " << b+1 << "/" << allBooks.size();
  }
  if(type == "train") dictionary.clear();
  clog << endl;
}

void Dataset::calculateComputedError(const vector<double> &output, const vector<int> &expectedResult, const double &decisionFactor){
  //calculate how good/bad was genre detection
      int genre1 = -1;//first detected genre
      int genre2 = -1;//second detected genre
      int error1 = 0;//false negative error
      int error2 = 0;//false positive error
      int missedGenre1 = -2;//fisrt missedcgenre
      int missedGenre2 = -2;//second missed genre
      int overOne = 0;//number of elements bigger tan decisionFactor
      int expectedGenre1 = -2;//genres that should have been detected
      int expectedGenre2 = -2;


    for (size_t d = 0; d < expectedResult.size(); d++) {
      if(expectedResult[d] == 1){ //detrmine expected genres
          if(expectedGenre1 == -2) expectedGenre1 = d;
          else expectedGenre2 = d;
      }
      if(expectedResult[d] == 1 && output[d] >= decisionFactor && genre1 == -1){
          overOne++; //if number is bigger than decision factor, we count it as detecetd genre
          genre1 = d;
      }
      if(expectedResult[d] == 1 && output[d] >= decisionFactor && genre1 > -1){
          overOne++;//if number is bigger than decision factor, we count it as detecetd genre
          genre2 = d;
        }
      if(expectedResult[d] == 1 && output[d] < decisionFactor) {
          error1++; //if genre was missed, we add false negative
          if(missedGenre1 == -2) missedGenre1 = d;
          if(missedGenre1 > -2) missedGenre2 = d;
      }
      if(expectedResult[d] == 0 && output[d] >= decisionFactor) {
          overOne++; //if program detected genre, where the is no genre, we add false positive
          error2++;
      }
  }
    if(overOne == 0){ //if all elements are smaller than decisionFactor, we take the biggest one as the right one
    double max = 0.0;
       for (size_t d = 0; d < output.size(); d++) {
           if(output[d] >= max) {
               genre1 = d;
               max = output[d];
           }
       }
        if(missedGenre1 == genre1 || missedGenre2 == genre1) error1--;
        else{
            error2++;
        }
    }

    if(overOne == output.size()){//if all elements are bigger than decisionFactor, we take the biggest one as the right one
    double max = 0.0;
       for (size_t d = 0; d < output.size(); d++) {
           if(output[d] >= max) {
               genre1 = d;
               max = output[d];
           }
       }
        error2 = 0;
        if(expectedGenre1 == genre1 || expectedGenre2 == genre1) error1--;
        else{
            error2++;
        }
    }

    if(overOne == output.size() - 1){
        double min = 100.0;
        for (size_t d = 0; d < output.size(); d++) {
            if(output[d] <= min) {
                if(genre1 > -1) error1++;
                genre1 = d;
                min = output[d];
            }
        }
        error2 = 0;
        if(missedGenre1 == genre1 || missedGenre2 == genre1) {
            error1--;
        }
        else{
            error2++;
        }
    }
//count all true positives/false negatives/false positives
    if(error1 < 1 && error2 < 1){
        truePositive++;
        genres[expectedGenre1].truePositive++;
        if(expectedGenre2 > -1)genres[expectedGenre2].truePositive++;
    }
    if(error1 > 0 && error2 < 1){
        falseNegative++;
        genres[missedGenre1].falseNegative++;
        if(missedGenre2 > -1)genres[missedGenre2].falseNegative++;
    }
    if(error2 > 0){
        falsePositive++;
        genres[expectedGenre1].falsePositive++;
        if(expectedGenre2 > -1)genres[expectedGenre2].falsePositive++;
    }
}

void Dataset::testDatasetNetwork(const string &modelname){

  truePositive = 0;
  falseNegative = 0;
  falsePositive = 0;

  for (size_t g = 0; g < genres.size(); g++) {
    genres[g].truePositive = 0;
    genres[g].falseNegative = 0;
    genres[g].falsePositive = 0;
  }


  double decisionFactor = 0.5; //a number, which we take as the lowest probability, after which genre will be detected

  auto start = std::chrono::system_clock::now();

  Net net(modelname);//create network
  net.readNetwork();//load pretrained model
  for (size_t b = 0; b < allBooks.size(); b++) {
    vector<double> computedGenre = net.getResult(allBooks[b]->bookVector);//feedForward input and get result
    clog << "\rBooks tested " << b + 1;
    calculateComputedError(computedGenre, allBooks[b]->genre, decisionFactor);//calculate how good/bad the result was
    if(allBooks[b]->computedGenres.find("Network") == allBooks[b]->computedGenres.end()){ //add result to Book
      allBooks[b]->computedGenres.insert({"Network", computedGenre});
    }
    else{
      auto itr = allBooks[b]->computedGenres.find("Network");
      itr->second = computedGenre;
    }
  }
  auto end = std::chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end - start;
  clog << endl << "computing time: " << elapsed_seconds.count() << endl;
  clog << endl;
  clog << "==========================" << endl;
  clog << "RESULTS NETWORK" << endl;
  clog << endl;
  clog << "BOOKS TESTED:    " << booksCount << endl;
  clog << "TRUE POSITIVE:   " << truePositive << endl;
  clog << "FALSE NEGATIVE:  " << falseNegative << endl;
  clog << "FALSE POSITIVE:  " << falsePositive << endl;
  clog << endl;
  double precision = (double)truePositive/(double)(truePositive+falsePositive);
  double recall = (double)truePositive/(double)(truePositive + falseNegative);
  double f1 = (2*precision*recall)/(precision+recall);
  clog << "F1 = " <<f1 << endl;
  double macroAverage = 0.0;
  vector<double> genreScores = {};
  for (size_t g = 0; g < genres.size(); g++) {
    double genrePrecision = (double)genres[g].truePositive/(double)(genres[g].truePositive+genres[g].falsePositive);
    double genreRecall = (double)genres[g].truePositive/(double)(genres[g].truePositive + genres[g].falseNegative);
    double genreF1 = (2*genrePrecision*genreRecall)/(genrePrecision+genreRecall);
    genreScores.push_back(genreF1);
    macroAverage += genreF1;
  }
  macroAverage = macroAverage/(double)genres.size();
clog << "AVG = " << macroAverage << endl;
  for (size_t g = 0; g < genres.size(); g++) {
    clog << endl << "GENRE " << genres[g].genreName << endl;
    clog << "F1 = " << genreScores[g] << endl;
    clog << "TRUE POSITIVE | FALSE NEGATIVE | FALSE POSITIVE" << endl;
    clog << setw(13) << genres[g].truePositive << " | ";
    clog << setw(14) << genres[g].falseNegative << " | ";
    clog << setw(14) << genres[g].falsePositive << endl;
  }

  clog << "==========================" << endl;
}


void Dataset::testBookNetwork(shared_ptr<Book> testbook, const string &modelName){
  Net net("model/"+modelName);//create network
  net.readNetwork();//load pretrained model
  vector<double> computedGenre = net.getResult(testbook->bookVector);//feedForward input and get result
  if(testbook->computedGenres.find("Network") == testbook->computedGenres.end()){ //add result to Book
    testbook->computedGenres.insert({"Network", computedGenre});
  }
  else{
    auto itr = testbook->computedGenres.find("Network");
    itr->second = computedGenre;
  }
}


void Dataset::testSingleBook(string &text, const Dataset &trainDataset, const string &modelName){
  string genre1 = " ";
  string genre2 = " ";
  int overOne = 0;
  shared_ptr<Book> b(new Book(booksCount+1000000));
  text = text+" ";
  b->fillWords(text);
  allBooks.push_back(b);
  booksCount++;
  fillBookVectors();
  int t = 1;
  testBookBayes(b, trainDataset, &t);
  testBookNetwork(b, modelName);
  cout << "==========================" << endl;
  cout << "RESULTS" << endl << endl;
  for(auto it = b->computedGenres.begin(); it != b->computedGenres.end(); it++){
    if(it->first == "Bayes"){
      for (size_t i = 0; i < it->second.size(); i++) {
        if(it->second[i] > 1.0){
          overOne++;
          if(genre1 == " ") genre1 = trainDataset.genres[i].genreName;
          else genre2 = trainDataset.genres[i].genreName;
        }
      }

      if(overOne == 0){ //if all elements are smaller than decisionFactor, we take the biggest one as the right one
      double max = 0.0;
         for (size_t d = 0; d < it->second.size(); d++) {
             if(it->second[d] >= max) {
                 genre1 = trainDataset.genres[d].genreName;
                 max = it->second[d];
             }
         }
       }
       if(overOne == it->second.size()){//if all elements are bigger than decisionFactor, we take the biggest one as the right one
       double max = 0.0;
       genre2 = " ";
          for (size_t d = 0; d < it->second.size(); d++) {
              if(it->second[d] >= max) {
                  genre1 = trainDataset.genres[d].genreName;
                  max = it->second[d];
              }
          }
       }
        if(overOne == it->second.size() - 1){
            double min = 100.0;
            genre2 = " ";
            for (size_t d = 0; d < it->second.size(); d++) {
                if(it->second[d] <= min) {
                    genre1 = trainDataset.genres[d].genreName;
                    min = it->second[d];
                }
            }
          }
        if((overOne > 2) && (overOne < (it->second.size() - 1))){
            genre2 = " ";
            genre1 = "Could not detect genre";
        }
          cout << "Bayes computet genre: " << genre1;
          if(genre2 != " ")cout << ", " << genre2;
          cout << endl;
          genre1 = " ";
          genre2 = " ";
          overOne = 0;
  }
  if(it->first == "Network"){
    for (size_t i = 0; i < it->second.size(); i++) {
      if(it->second[i] > 0.5){
        overOne++;
        if(genre1 == " ") genre1 = trainDataset.genres[i].genreName;
        else genre2 = trainDataset.genres[i].genreName;
      }
    }
    if(overOne == 0){ //if all elements are smaller than decisionFactor, we take the biggest one as the right one
    double max = 0.0;
       for (size_t d = 0; d < it->second.size(); d++) {
           if(it->second[d] >= max) {
               genre1 = trainDataset.genres[d].genreName;
               max = it->second[d];
           }
       }
     }
     if(overOne == it->second.size()){//if all elements are bigger than decisionFactor, we take the biggest one as the right one
     double max = 0.0;
     genre2 = " ";
        for (size_t d = 0; d < it->second.size(); d++) {
            if(it->second[d] >= max) {
                genre1 = trainDataset.genres[d].genreName;
                max = it->second[d];
            }
        }
      }
      if(overOne == it->second.size() - 1){
          double min = 100.0;
          genre2 = " ";
          for (size_t d = 0; d < it->second.size(); d++) {
              if(it->second[d] <= min) {
                  genre1 = trainDataset.genres[d].genreName;
                  min = it->second[d];
              }
          }
        }
        cout << "Network computet genre: " << genre1;
        if(genre2 != " ")cout << ", " << genre2;
        cout << endl;
        genre1 = " ";
        genre2 = " ";
        overOne = 0;
  }
  }
  cout << "==========================" << endl;
  allBooks.clear();
}
