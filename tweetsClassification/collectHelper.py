"""
collect.py: This should collect data used in your analysis.
This may mean submitting queries to Twitter or Facebook API, or scraping webpages.
The data should be raw and come directly from the original source -- that is, you may not use data that others have already
collected and processed for you (e.g., you may not use SNAP datasets).
Running this script should create a file or files containing the data that you need for the subsequent phases of analysis.
"""
import configparser
import csv
import re
import sys
import time
from collections import defaultdict
import tweepy
from TwitterAPI import TwitterAPI
from tweepy import OAuthHandler


class collect():

    def __init__(self, screenName, limit, credentialFileName, trainingTwitterHandles, testingTwitterHandles):
        self.screenName = screenName
        self.limit = limit
        self.credentialFileName = credentialFileName
        self.trainingTwitterHandles = trainingTwitterHandles
        self.testingTwitterHandles = testingTwitterHandles

    def getCredentials(self):
        """
        Gets Credentials from File
        :return:
        CONSUMER_KEY, CONSUMER_SECRET, ACCESS_TOKEN, ACCESS_TOKEN_SECRET
        """
        config = configparser.ConfigParser()
        config.read(self.credentialFileName)
        CONSUMER_KEY = config.get('twitter', 'consumer_key')
        CONSUMER_SECRET = config.get('twitter', 'consumer_secret')
        ACCESS_TOKEN = config.get('twitter', 'access_token')
        ACCESS_TOKEN_SECRET = config.get('twitter', 'access_token_secret')
        credentials = (CONSUMER_KEY, CONSUMER_SECRET, ACCESS_TOKEN, ACCESS_TOKEN_SECRET)

        return credentials

    def get_twitter_API(self):
        """
        Construct an instance of TwitterAPI using the tokens you entered above.
        :return:
         An instance of TwitterAPI
        """
        consumer_key, consumer_secret, access_token, access_token_secret = self.getCredentials()
        return TwitterAPI(consumer_key, consumer_secret, access_token, access_token_secret)

    def get_tweepy_API(self):
        """
        Construct an instance of TweepyAPI using the tokens you entered above.
        :return:
         An instance of TweepyAPI
        """
        consumer_key, consumer_secret, access_token, access_token_secret = self.getCredentials()
        auth = OAuthHandler(consumer_key, consumer_secret)
        auth.set_access_token(access_token, access_token_secret)
        return tweepy.API(auth)

    def robust_request(self, twitter, resource, params, max_tries=10):
        """ If a Twitter request fails, sleep for 15 minutes.
        Do this at most max_tries times before quitting.
        Args:
          twitter .... A TwitterAPI object.
          resource ... A resource string to request; e.g., "friends/ids"
          params ..... A parameter dict for the request, e.g., to specify
                       parameters like screen_name or count.
          max_tries .. The maximum number of tries to attempt.
        Returns:
          A TwitterResponse object, or None if failed.
        """
        for i in range(max_tries):
            request = twitter.request(resource, params)
            if request.status_code == 200:
                return request
            elif (request.status_code != 401):
                print('Got error %s \nsleeping for 15 minutes.' % request.text)
                sys.stderr.flush()
                time.sleep(61 * 15)

    def get_followers(self, screen_name, key):
        """
         Return a list of Twitter IDs for users that this person is being follwed by, up to 5000..
    Args:
        twitter.......The TwitterAPI object
        screen_name... a string of a Twitter screen name
    Returns:
        A list of ints, one per follower ID
        :param limit:
        :param screen_name:
        :param key:
        :return:
        """
        follower_list = []
        response = self.robust_request(self.get_twitter_API(), "followers/ids",
                                       {key: screen_name, 'stringify_ids': 'true', 'count': self.limit})
        if (response != None):
            for followers in response:
                follower_list.append(followers)

        return sorted(follower_list)

    def add_all_followers(self):
        """
        Collects the "limit" number of followers for the screenname and further, collects "limit" number of followers for the followers of the ScreenName
        :return:
         A dict of ScreenNames/User_ID mapped to their "limit" number of followers
        """

        follower_ids = defaultdict(list)
        sub_follower_ids = defaultdict(list)

        response = self.get_followers(self.screenName, 'screen_name')

        for follower_id in response:
            follower_ids[self.screenName].append(follower_id)

        for k, v in follower_ids.items():
            for ids in v:
                response = self.get_followers(ids, 'user_id')
                for follower_id in response:
                    sub_follower_ids[ids].append(follower_id)

        for key, value in sub_follower_ids.items():
            follower_ids[key] = value

        return follower_ids

    def writeToCSV(self,fileName,tweet,screenName,mode):
        csvFile = open(fileName, 'a')
        csvWriter = csv.writer(csvFile)
        if(mode=='training'):
            csvWriter.writerow((self.trainingTwitterHandles[screenName], tweet))
        elif(mode=='testing'):
            csvWriter.writerow((self.testingTwitterHandles[screenName],tweet))
        csvFile.close()

    def processTweet(self,fileName,list_of_tweets,screenName,mode):

        for tweet in list_of_tweets:
            text = tweet
            self.writeToCSV(fileName+".csv",text,screenName,mode)

    def cleanTweet(self,text):

        text = re.sub('http\S+', 'this_is_an_url', text)
        text = re.sub('www\S+', 'this_is_an_url', text)
        text = re.sub('@\S+', 'this_is_a_mention', text)
        text = re.sub('[\s]+', ' ', text)
        text = re.sub("\n+", " ", text)
        text = re.sub(r'#([^\s]+)', r'\1', re.sub('[\s]+', ' ', text)).strip('\'"')

        return text

    def collectTweets(self,mode):
        """
        Collect Tweets of the User for both training and testing Files
        :param mode: "Training""Testing"
        :return:
        """

        self.limit = defaultdict(int)
        api = self.get_tweepy_API()

        if(mode=='training'):
            tweetHandles = self.trainingTwitterHandles
        elif(mode=='testing'):
            tweetHandles = self.testingTwitterHandles

        for screenName,handle in tweetHandles.items():
            list_of_tweets = []
            tweetsList = []
            tweets = api.user_timeline(screen_name=screenName, count=200,include_rts = False)
            list_of_tweets.extend(tweets)
            for tweet in tweets:
                text = self.cleanTweet(tweet.text)
                if (('RT @' not in str(text)) and ('RT ' not in str(text))) and (str(text) not in tweetsList):
                    tweetsList.append(text)
                    self.limit[screenName] += 1
            oldest_tweet = list_of_tweets[-1].id - 1
            while (len(tweets) > 0 and self.limit[screenName] < 800):
                tweets = api.user_timeline(screen_name=screenName, count=200,max_id=oldest_tweet,include_rts = False)
                list_of_tweets.extend(tweets)
                for tweet in tweets:
                    text = self.cleanTweet(tweet.text)
                    if (('RT @' not in str(text)) and ('RT ' not in str(text))) and (str(text) not in tweetsList):
                        tweetsList.append(text)
                        self.limit[screenName] += 1
                oldest_tweet = list_of_tweets[-1].id - 1

            self.processTweet(mode, tweetsList, screenName,mode)
