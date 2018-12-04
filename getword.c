/*	getword.c
	CS570 Fall 2016
	San Diego State University
	Professor Carroll
	Robert Brobst Jr
	Due September 7, 2016
	
	Template from inout2.c in CbyDiscovery
 */

/* Include Files */
#include <stdio.h>
#include "getword.h"
#include "p2.h"

int getword (char *w)
{
    int special_char = 0; //0 if no '\' character, 1 if there is a '\' character
    int negate_word = 0; //1 if make char amount negative, 0 if positive
    int iochar;
    int counter = 0; // counter for length of word (character counter)
    char *start = w; //saves the start of the word
   
    while ( ( iochar = getchar() ) != EOF )
    {
        /* First, the loop checks if the character amount is over STORAGE - 1.
	If it is, it will null terminate the word, use ungetc on the current
	character and return the counter. If negate_word is flagged with a '$',
	then it will make counter a negative value.
	*/
	if (counter >= STORAGE - 1)
	{
	    *w = '\0';
	    ungetc(iochar,stdin);
	    if (negate_word == 1)
	    {
		counter = counter * -1;
	    }
	    return counter;
	}
	
	/* The loop then checks for spaces. If there have been no characters
	counted yet, then it will skip the iteration of the loop as these are
	considered leading spaces. If the special_char '\' is not flagged
	(special_char = 0), it will null terminate the word and return the 
	character count. If negate_word is flagged (negate_word = 1), it will 
	make the character count negative before returning. If special_char is 
	flagged (special_char = 1), it will set special_char to 0 and continue 
	the loop.
	*/
        if (iochar == ' ')
	{
	    if (counter == 0)
	    {
	    	continue;
	    }
	    if (special_char == 0)
	    {
		*w = '\0';
		if (negate_word == 1)
		{
		    counter = counter * -1;
		}
		return counter;
	    }
	    else if (special_char == 1)
	    {
	        special_char = 0;
	    }
	}
	
	/* The loop then checks for line. If there have been no characters
	counted yet, then it will set the word to null and return 0. Otherwise, 
	it will null terminate the word, ungetc the newline, and return the 
	character count. If negate_word is flagged, it will make the character 
	count negative before returning.
	*/
	if (iochar == '\n')
	{ 
	    if (counter == 0)
	    {
	    	*start = '\0';
	        return 0;
	    }
	    
	    *w = '\0';
	    ungetc(iochar,stdin);
	    if (negate_word == 1)
	    {
		counter = counter * -1;
	    }
	    return counter;
	}
	
	/*Then the loop checks for a '$'. If found and there have been no
	characters counted yet, and special_char has not been flagged, which
	means there isn't a '\' before the '$', then negate_word will be set to
	one which means it is flagged. Nothing else is needed since in other
	situations, '$' follows normal rules.
	*/
	if (iochar == '$')
	{
	    if (counter == 0)
	    {
	        if (special_char == 0)
		{
	            negate_word = 1;
		}
	    }
	}
	
	/* Then the loop checks if the character is a '\'. If it is and
	special_char is not flagged, it will flag special_char and skip the rest
	of the iteration of the loop. If it already flagged, it will unflag
	special_char.
	*/
	if (iochar == '\\')
	{
	    if (special_char == 0)
	    {
	    special_char = 1;
	    special_flag = 1; //global variable to know about '\\'
	    continue;
	    }
	    else if (special_char == 1)
	    {
	    special_char = 0;
	    }
	}
	
	/* Then, the loop will check for metacharacters. If any metacharacters
	are found, it will check if special_char has not been flagged. If it
	hasn't, then it will check if there hasn't been any characters counted
	yet. If not, it will place the metacharacter into the string, and then
	null terminate it. Then, it will set the character counter to 1 and if
	negate_word is flagged, it will make the counter negative. Then it will
	return the counter.
	
	If the character counter has counted characters, it will null terminate
	the current word, ungetc the current character, do the negative check
	and return the character counter.
	
	If the special character has been flagged however, it will unflag
	special_char. This is because the metacharacter will just be treated as
	a normal character so nothing needs to be done except to show that the
	special character has been dealt with.
	*/
	if (iochar == '<' || iochar == '>' || iochar == '|' || iochar == '&')
	{
	    if (special_char == 0)
	    {
		if (counter == 0)
		{
		    *w++ = iochar;
		    *w = '\0';
		    counter = 1;
		    if (negate_word == 1)
		    {
			counter = counter * -1;
		    }
		    return counter;
		}

		*w = '\0';
		ungetc(iochar,stdin);
		if (negate_word == 1)
		{
		    counter = counter * -1;
		}
		return counter;
	    }
	    else if (special_char == 1)
	    {
	        special_char = 0;
	    }
	}
	
	/* Finally, it will check if special character is still flagged. If it
	is, it will unflag. This scenario happens if the special character
	wasn't unflagged from the above characters. Such as, '\k' or '\j' will
	end here.
	*/
	if (special_char == 1)
	{
	    special_char = 0;
	}
	
	/* If the character count hasn't returned by this point, it will add the
	character to the string and increment the w and the character count.
	*/
	*w++ = iochar;	  
	counter++;
    }
    
    /* If the loop encounters EOF in any circumstance, it will be sent here and
    set the null terminate the word. If the counter is at 0 which means there is
    no word, it will return -1. Otherwise it will return the length of the final
    word and then return -1 when getword is called again.
    */
    *w = '\0';
    if (counter == 0)
    {
        counter = -1;
    }
    
    return counter;
}

