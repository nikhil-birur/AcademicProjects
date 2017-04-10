"""
summarize.py: This should read the output of the previous methods to write a textfile called summary.txt containing the following entries:

Number of users collected:
Number of messages collected:
Number of communities discovered:
Average number of users per community:
Number of instances per class found:

One example from each class:
"""
import itertools
import pandas as pd
from classifyHelper import classify
from collectHelper import collect
from clusterHelper import cluster
import networkx as nx
import csv



class summarizerHelper():

    def __init__(self,mode):
        self.mode = mode


    def cluster_data(self):

        #helper function to call the mthods in the Cluster Class

        credentialFileName = "twitter.cfg"
        trainingTwitterHandles = ""
        testingTwitterHandles = ""
        screen_name = "FoxNews"
        limit = 100
        networkFileName = "network.png"
        G = nx.Graph()

        col = collect(screen_name,limit,credentialFileName,trainingTwitterHandles,testingTwitterHandles)
        follower_ids = col.add_all_followers()
        clu = cluster(follower_ids,networkFileName,G)
        G = clu.draw_network()
        clusters = clu.girvan_newman(G)

        return(clusters,G)


    def classify_data(self):

        # helper function to call the mthods in the Cluster Class

        trainingFileName = "training.csv"
        testingFileName = "testing.csv"
        outPutFileName = "prediction.csv"

        cla = classify(trainingFileName,testingFileName,outPutFileName)
        cla.NaiveBayesClassifier()

        return outPutFileName,trainingFileName


    def main(self):

        # Cluster Methods
        if(self.mode=="cluster"):
            csvFile = open("clusterSummary.csv", 'a')
            csvWriter = csv.writer(csvFile)
            clusters = tuple
            number_of_clusters = 15
            cluster = self.cluster_data()
            limited = itertools.takewhile(lambda c: len(c) <= number_of_clusters, cluster[0])
            for communities in limited:
                clusters = tuple(c for c in communities)
            csvWriter.writerow((
                "Number of Users Collected :" , cluster[1].number_of_nodes()))
            csvWriter.writerow((
                "Number of Communities discovered :", len(clusters)))
            csvWriter.writerow((
                "Number of clusters asked for :", number_of_clusters))
            csvWriter.writerow((
                "Average Number of Users per community :" , (cluster[1].number_of_nodes()/number_of_clusters)
            ))
            print("Communities are:")
            print("--------------------------------------------------")
            for community in clusters:
                print(community)

            print("\n\n\n")


            csvFile.close()



        # Classify Methods

        elif(self.mode=="classify"):
            csvFile = open("classifySummary.csv", 'a')
            csvWriter = csv.writer(csvFile)
            predictedFile,trainingFile = self.classify_data()
            csvFile = open(trainingFile, 'r')
            wrongPred = 0
            number_of_trainTweets = sum(1 for row in csvFile)
            predictions = pd.read_csv("prediction.csv",
                             header=None,
                             names=['prediction', 'truth', 'tweet'])

            rightInstances = {'sports':'','science':"",'business':""}
            wrongInstances = {'sports':'','science':"",'business':""}

            for index,row in predictions.iterrows():
                if(row['prediction']!=row['truth']):
                    wrongPred += 1
                    if(row['truth']=='Sports'):
                        wrongInstances['sports'] = row['prediction']+" "+row['truth']+" "+row['tweet']
                    elif(row['truth']=='Business'):
                        wrongInstances['business'] = row['prediction']+" "+row['truth']+" "+row['tweet']
                    elif(row['truth']=='Science'):
                        wrongInstances['science'] = row['prediction']+" "+row['truth']+" "+row['tweet']
                else:
                    if (row['truth'] == 'Sports'):
                        rightInstances['sports'] = row['prediction'] + " " + row['truth'] + " " + row['tweet']
                    elif (row['truth'] == 'Business'):
                        rightInstances['business'] = row['prediction'] + " " + row['truth'] + " " + row['tweet']
                    elif (row['truth'] == 'Science'):
                        rightInstances['science'] = row['prediction'] + " " + row['truth'] + " " + row['tweet']

            csvWriter.writerow(
                ("Number of Tweets Collected" , (predictions.shape[0]+number_of_trainTweets)))
            csvWriter.writerow((
                "Total Number of wrong predictions", wrongPred))
            testing_tweets = predictions.shape[0]
            csvWriter.writerow((
                "Accuracy", 100 - ((wrongPred/testing_tweets)*100)))
            csvWriter.writerow((
                "Number of predicted Instances\n" , (pd.value_counts(predictions['prediction'].values, sort=True).to_string())))
            csvWriter.writerow((
                "Number of True Instances\n",
                (pd.value_counts(predictions['prediction'].values, sort=True).to_string())))

            print("****************************************************************************************************")
            print("Examples\n")
            print("Instances of right predcitions: ")
            print("--------------------------------------------------")
            print("|Predicted| |Truth| |Tweet|")
            for k,v in rightInstances.items():
                print(v)
            print("\n")
            print("Instances of wrong predcitions: ")
            print("--------------------------------------------------")
            print("|Predicted| |Truth| |Tweet|")
            for k,v in wrongInstances.items():
                print(v)
            print("\n")
            print("****************************************************************************************************")

            csvFile.close()
