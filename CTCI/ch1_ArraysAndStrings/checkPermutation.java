/*
	Check Permutation : Given two strings, write a method 
	to decide if one is a permutaion of another.
*/


import java.util.*;

public class checkPermutation{

	public String sortString(String str){
		char[] c = str.toCharArray();
		Arrays.sort(c);
		return new String(c);
	}

	public boolean ischeckPerm1(String[] str){
		String str1 = str[0].trim().toLowerCase();;
		String str2 = str[1].trim().toLowerCase();;
		if(str1.length()!=str2.length())return false;
		return sortString(str2).equals(sortString(str1));
	}

	public boolean ischeckPerm2(String[] str){
		String str1 = str[0].trim().toLowerCase();
		String str2 = str[1].trim().toLowerCase();
		HashMap<Character,Integer> hm = new HashMap<Character,Integer>();
		for(char c : str1.toCharArray()){
			hm.put(c,hm.getOrDefault(c,0)+1);
		}
		for(char c : str2.toCharArray()){
			if(!hm.containsKey(c))return false;
			hm.put(c,hm.get(c)-1);
			if(hm.get(c)>0)return false;
		}
		return true;
	}

	public boolean ischeckPerm3(String[] str){
		String str1 = str[0].trim().toLowerCase();
		String str2 = str[1].trim().toLowerCase();
		int[] char_count = new int[128];
		for(char c : str1.toCharArray()){
			char_count[c]++;
		}
		for(char c : str2.toCharArray()){
			char_count[c]--;
			if(char_count[c]<0)return false;
		}


		return true;
	}

	public static void main(String args[]){
		String[] str = {"dog          ", "gOd"};
		checkPermutation uni = new checkPermutation();
		System.out.println(uni.ischeckPerm1(str)+" "+uni.ischeckPerm2(str)+" "+uni.ischeckPerm3(str));


	}

	
}