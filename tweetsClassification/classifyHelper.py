"""
classify.py: This should classify your data along any dimension of your choosing (e.g., sentiment, gender, spam, etc.).
You may write any files you need to save the results.
"""
import csv
import re
import string

import nltk


class classify():

    def __init__(self,trainingFileName,testingFilename,outPutFileName,featureList = []):
        self.trainingFileName = trainingFileName
        self.testingFilename = testingFilename
        self.outPutFileName = outPutFileName
        self.featureList = featureList


    def getStopWordList(self,filename):
        stopWords = ['this_is_a_mention', 'this_is_an_url']
        with open(filename) as infile:
            for line in infile:
                stopWords.append(line.strip())
        return stopWords

    def extract_features(self,tweet):
        tweet_words = set(tweet)
        vector = {}
        for word in self.featureList:
            if word in tweet_words:
                vector['feature = (%s)' % word] = 1
            else:
                vector['feature = (%s)' % word] = 0

        return vector

    def vectorize(self,tweet,stopWords):
        vector = []
        words = (re.sub(r'\s+', " ", ' '.join(d.strip(string.punctuation.replace("_", "")) for d in (re.sub('[^\w]', ' ', tweet.strip(' ').lower())).split())).lower()).split()
        for w in words:
            pattern = re.compile(r"(.)\1{1,}", re.DOTALL)
            val = re.search(r"^[a-zA-Z][a-zA-Z0-9]*$", pattern.sub(r"\1\1", re.sub("[0-9]", "", w)))
            if (w in stopWords or val is None):
                continue
            else:
                vector.append(w.lower())
        return vector

    def testTweets(self,NBClassifier,stopWords):
        csvFile = open(self.outPutFileName, 'w')
        csvWriter = csv.writer(csvFile)
        testTweets = csv.reader(open(self.testingFilename, 'r'), delimiter=',')
        for row in testTweets:
            testTweet = row[1]
            sentiment = NBClassifier.classify(self.extract_features(self.vectorize(testTweet.lower(), stopWords)))
            csvWriter.writerow(
                [sentiment,row[0] ,testTweet])
        csvFile.close()


    def NaiveBayesClassifier(self):
        trainedTweets = csv.reader(open(self.trainingFileName, 'r'), delimiter=',')
        stopWords = self.getStopWordList('stopwords.txt')
        tweets = []
        for row in trainedTweets:
            sentiment = row[0]
            tweet = row[1]
            featureVector = self.vectorize(tweet.lower(), stopWords)
            self.featureList.extend(featureVector)
            tweets.append((featureVector, sentiment))
        training_set = nltk.classify.util.apply_features(self.extract_features, tweets)
        NBClassifier = nltk.NaiveBayesClassifier.train(training_set)
        self.testTweets(NBClassifier,stopWords)
