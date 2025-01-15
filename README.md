# GenreGuesser

A supervised learning program for classifying books’ genres based on their descriptions. It uses two different approaches:

1. **Naive Bayes**  
2. **Neural Network** (back propagation learning algorithm)

For more information about the methods used to classify books in this program, please see the **INFO.txt** file.

---

## Important Notes

- **❗️** – Important information  
- **++** – Information about sub-arguments  

---

## Compilation

1. Open your terminal and change the directory to `Path/To/GenreGuesser`.
2. Run:
   ```bash
   make -f makefile.mk
   ```
3. The resulting executable will be named **`genreDetector`**.

**❗️ Warning:** The word2vec dictionary used for the Neural Network is very large (~3.5 GB). Only a preview version of that dictionary (located at `dataset/datasetsrc/wordvectors.txt`) is part of this repository. Because of this:
- Training new models on your own data is **not** possible with the preview dictionary.
- Single-text tests requiring the entire dictionary are also **not** possible.

---

## Program Arguments

Below is a list of the arguments supported by `genreDetector` and their respective behaviors.

### `--newDataset`

Use this argument if you have a new dataset source file and want to create a new train/test dataset from it.

#### What it does

- Reads source files (`traindatasrc.txt` / `testdatasrc.txt`) from `dataset/datasetsrc/`.
- Creates a new dataset file for faster access.
- **Required format** for each book in the source file:
  ```xml
  <body>book's description text</body>
  <topics>
    <d0>Genre1</d0><d0>Genre2</d0>
  </topics>
  ```

#### Sub-Arguments

- **`-test`**  
  Creates a new **test** dataset.
- **`-train`**  
  Creates a new **train** dataset.
- **No sub-argument**  
  Creates **both** train and test datasets.

#### Warnings

- **❗️** Creating new datasets is **slow** and can take several hours.  
- **❗️** Up to **4GB of RAM** might be required. Make sure you have sufficient memory before starting.

---

### `--trainModel`

Use this argument if you want to **train an existing Neural Network model**.

#### Inputs Prompted

1. **Model’s name**  
   - The full name of the model (e.g., `modelA_B_C.txt`).
2. **Learning rate**  
   - Typical values: `0.1`, `0.01`, `0.001`.  
   - See **INFO.txt** (LINKS section) for more details.
3. **Momentum**  
   - Typically greater than `0.8`.  
   - Adds a fraction of the previous training step to the new one.  
   - Also see **INFO.txt** (LINKS section).
4. **Size of batches**  
   - How many samples are back-propagated before the weights are updated.  
   - Larger batches = fewer updates per epoch, but potentially less accuracy.

#### Sub-Argument: `-new`

- If you want to **create a new model** instead of training an existing one.  
- You will be **additionally asked** for the model’s **topology**:  
  - Example: `100 20 10 7`  
  - The number of digits = number of neuron layers (4 layers in this example).  
  - Each digit = number of neurons in that layer (100 in the first layer, 7 in the last layer).

#### Training Process

- After training starts, you will see:  
  - **Average error** per epoch  
  - **Time** needed for the computations

---

### `--test`

Use this argument if you want to **test** the performance of the learning methods on book descriptions.

#### No Sub-Arguments

- Tests **both** Bayes and Neural Network methods **on the training dataset**.  
- **Result Table** will include:

  - **BOOKS TESTED** – number of tested books  
  - **TRUE POSITIVE** – number of books whose genres were correctly determined  
  - **FALSE NEGATIVE** – books assigned two genres, but the program only detected one  
  - **FALSE POSITIVE** – program incorrectly determined one or more genres  
  - **F1 score** – overall micro-average score  
  - **AVG score** – average F1 score across all genres

- Statistics for each genre are also displayed, containing the same metrics.

> **Note:** The sum of True Positives, False Negatives, and False Positives for each genre may not match the total number of books in that genre because some samples are assigned multiple genres.

#### `-bayes`

- Test **only** the Bayes method.  
- **❗️** Because training is slow, you will be asked how many books you want to test.

#### `-network`

- Test **only** the Neural Network method.

#### `-single`

- Test the algorithms on **your own texts**.  
- **❗️ Warning:** This requires loading the entire word-vector dictionary (~3.5 GB) into RAM, which is not included in this repository except for a small preview.

---

## Final Notes

- For more details, including helpful links and deep-dives into methodology, please check the **INFO.txt** file.  
- Make sure you have **enough disk space** (the word-vector file can be very large).  
- Make sure you have **enough free RAM** if you attempt training or single-text tests with the full dictionary.
