static int
StringFirstCmd (
	ClientData dummy,		/* Not used. */
	Tcl_Interp *interp,		/* Current interpreter. */
	int objc,			/* Number of arguments. */
	Tcl_Obj *const objv[] )	/* Argument objects. */
{
	Tcl_UniChar *ustring1, *ustring2;
	int match, start, length1, length2;

	if ( objc < 3 || objc > 4 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv,
						   "needleString haystackString ?startIndex?" );
		return TCL_ERROR;
	}

	/*
	 * We are searching string2 for the sequence string1.
	 */

	match = -1;
	start = 0;
	length2 = -1;

	ustring1 = Tcl_GetUnicodeFromObj ( objv[1], &length1 );
	ustring2 = Tcl_GetUnicodeFromObj ( objv[2], &length2 );

	if ( objc == 4 )
	{
		/*
		 * If a startIndex is specified, we will need to fast forward to that
		 * point in the string before we think about a match.
		 */

		if ( TclGetIntForIndexM ( interp, objv[3], length2 - 1, &start ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		/*
		 * Reread to prevent shimmering problems.
		 */

		ustring1 = Tcl_GetUnicodeFromObj ( objv[1], &length1 );
		ustring2 = Tcl_GetUnicodeFromObj ( objv[2], &length2 );

		if ( start >= length2 )
		{
			goto str_first_done;
		}

		else if ( start > 0 )
		{
			ustring2 += start;
			length2 -= start;
		}

		else if ( start < 0 )
		{
			/*
			 * Invalid start index mapped to string start; Bug #423581
			 */

			start = 0;
		}
	}

	/*
	 * If the length of the needle is more than the length of the haystack, it
	 * cannot be contained in there so we can avoid searching. [Bug 2960021]
	 */

	if ( length1 > 0 && length1 <= length2 )
	{
		register Tcl_UniChar *p, *end;

		end = ustring2 + length2 - length1 + 1;

		for ( p = ustring2;  p < end;  p++ )
		{
			/*
			 * Scan forward to find the first character.
			 */

			if ( ( *p == *ustring1 ) && ( TclUniCharNcmp ( ustring1, p,
										  ( unsigned long ) length1 ) == 0 ) )
			{
				match = p - ustring2;
				break;
			}
		}
	}

	/*
	 * Compute the character index of the matching string by counting the
	 * number of characters before the match.
	 */

	if ( ( match != -1 ) && ( objc == 4 ) )
	{
		match += start;

	}

str_first_done:
	Tcl_SetObjResult ( interp, Tcl_NewIntObj ( match ) );
	return TCL_OK;
}
