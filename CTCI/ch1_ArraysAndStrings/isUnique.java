/*
	Is Unique : Imaplement an algorithm to determine if a string has all unique characters. What if you cannot use additional data structures?	
*/

import java.util.*;

public class isUnique{


	public boolean isStringUnique1(String str){
		if(str.length() > 128)return false;
		HashSet<Character> hs = new HashSet<Character>();
		char[] ch = str.toCharArray();
		for(char c : ch){
			if(hs.contains(c)){
				return false;
			}
			hs.add(c);
		}
		return true;
	}

	public boolean isStringUnique2(String str){
		if (str.length() > 128) {
			return false;
		}
		boolean[] char_set = new boolean[128];
		for (int i = 0; i < str.length(); i++) {
			int val = str.charAt(i);
			if (char_set[val]) return false;
			char_set[val] = true;
		}
		return true;
	}

	public boolean isStringUnique3(String str){
		if (str.length() > 128) {
			return false;
		}
		int checker = 0;
		for (int i = 0; i < str.length(); i++) {
			int val = str.charAt(i);
			int mask = 1 << val;
			System.out.println(Integer.toBinaryString(mask)+" "+Integer.toBinaryString(checker));
			if ((checker & mask) > 0) return false;
			checker |= mask;
		}
		return true;
		
	}



	public static void main(String args[]){
		Scanner sc = new Scanner(System.in);
		System.out.println("Enter the String:");
		String str = sc.nextLine();
		isUnique uni = new isUnique();
		System.out.println("Using HashSet: "+uni.isStringUnique1(str)+"\nUsing Arrays: "+
							uni.isStringUnique2(str)+"\nWithout any Data Structure: "+uni.isStringUnique3(str));


	}

	
}