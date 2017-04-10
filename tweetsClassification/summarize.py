import csv

def main():


    cluster = csv.reader(open("clusterSummary.csv", 'r'), delimiter=',')
    for row in cluster:
        print("\n")
        print(row[0])
        print("-----------------------------------------")
        print(row[1])

    classify = csv.reader(open("classifySummary.csv", 'r'), delimiter=',')
    for row in classify:
        print("\n")
        print(row[0])
        print("-----------------------------------------")
        print(row[1])




if __name__ == "__main__":
    main()
