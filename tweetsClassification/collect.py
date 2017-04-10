from collectHelper import collect

def main():
    credentialFileName = "twitter2.cfg"
    trainingFileName = "training.csv"
    testingFileName = "testing.csv"
    outPutFileName = "prediction.csv"
    classifyFileName = "classifySummary.csv"
    clusterFileName = "clusterSummary.csv"

    trainingTwitterHandles = {'@science': "Science", '@business': "Business", '@espn': "Sports"}
    testingTwitterHandles = {'@ScienceNews': "Science", '@businessinsider': "Business", '@SportsNation': "Sports"}
    screen_name = ""
    limit = None

    open(testingFileName, 'w').close()
    open(trainingFileName, 'w').close()
    open(outPutFileName, 'w').close()
    open(classifyFileName, 'w').close()
    open(clusterFileName, 'w').close()


    col = collect(screen_name, limit, credentialFileName, trainingTwitterHandles, testingTwitterHandles)
    col.collectTweets("training")
    col.collectTweets("testing")

if __name__ == "__main__":
    main()
