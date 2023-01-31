genreDetector README.md

This is a supervised learning program for classifying books’ genres based on their descriptions. It is using two different approaches: naive Bayes and neural network with back propagation learning algorithm.\

For more infrormation about the methods used to classify books in this program please see the INFO.txt file.\

*    - important information\
++  - information about sub arguments\
----------------------------------------

To compile this program change directory in your terminal to /Path/To/genreDetector and than use "make -f makefile.mk" command. The genreDetector program will be created.

* Warning: since the word2vec dictionary used for neural network is very large (3.5 GB), only a preview version of that dictionary is part of this repository (/dataset/datasetsrc/wordvectors.txt). This means that training of new models and single tests are not possible

The program accepts following arguments:

----------------------------------------
1) "--newDataset"\

Use this argument, if you have a new dataset source file, which you want to use for training. 

What it does: Program takes source files ("traindatasrc.txt/testdatasrc.txt) from dates/datasetsrc directory and creates new dataset file for faster access to needed information. Each book sample in the source file has to be in the following form:

//\
<body>book's description text</body>\
<topics>\
<d0>Genre1</d0><d0>Genre2</d0>\
</topics>\
//

++ Depending on sub arguments "-test"/"-train", the program will create new test/train dataset. If no sub argument is used, both datasets will be created

* Warning: process of creating is relatively slow and can take up to several hours!
* Warning: at some point the program will need up to 4GB of RAM, so free your memory before using

----------------------------------------
2) "--trainModel"

++ Use this argument if you want to train an already existing neural network model

You will be asked for the following inputs:

Model's name - this is the model name you would like to train. Please type in the full name of the model. (Mostly the names are modelA_B_C.txt, where A_B_C represent the topology used).

Learning rate - this parameter represents, how big steps will the learning algorithm be taking (typically used parameters are 0.1/0.01/0.001). For more information check LINKS section in INFO.txt file.

Momentum - this parameter represents, how big part of the previous learning step will be added to the new one (typically used parameters are bigger than 0.8). For more information check LINKS section in INFO.txt file.

Size of batches - represents how many samples will be back propagated before the weights will be updated. (The bigger the batches, the less time will an epoch take, but the less accurate the result will be).

++ If you want to create a new model, use sub argument "-new". You will be additionally asked for the model's topology.

Topology - this parameter looks like this: 100 20 10 7. Number of digits represents number of neutron layers [4] and each digit represents number of neurons in this concrete layer (100 neurons in the first layer and 7 in the last one)

After the learning begins, after each epoch you will see an average error for this epoch and time needed for the computations.

----------------------------------------
3) "--test"

Use this argument if you want to test how the learning methods perform on the new books's descriptions.

++ If you want to test both Bayes and neural network methods on the training dataset, use no sub arguments.

In the result table you will see the following statistics table:

BOOKS TESTED - number of books from the dataset, that were tested
TRUE POSITIVE - number of books, which genres were correctly determined
FALSE NEGATIVE - number of books, which were assigned two genres and only one of them was detected by the program
FALSE POSITIVE - the program incorrectly determined one or more genres

F1 score - overall micro average score
AVG score - averaged F1 score for each genre

Then the statistics for each genre follows containing the same parameters.

Notice, that the number of true positive, false negative and false positive samples in each genre don't sum up to the total number of books in that particular genre because of some samples being assigned to the multiple genres at once.


++ If you want to specifically test Bayes, use "-bayes" sub argument.

* Because the training process is slow and the whole dataset can take hours, you will be asked for the number of books you want to test.

++ If you want to specifically test Network, use "-network" sub argument.

++ If you want to test how the algorithms perform on your own texts, use "-single" sub argument.

* Warning: these single tests will need the whole word-vector dictionary to be loaded into PC's RAM and will need 3,5GB of free memory!










