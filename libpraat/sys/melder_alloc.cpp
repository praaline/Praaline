/* melder_alloc.cpp
 *
 * Copyright (C) 1992-2011,2014,2015 Paul Boersma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * pb 2002/03/07 GPL
 * pb 2006/12/10 separated from melder.c
 * pb 2007/05/24 wcsdup, wcsToAscii, asciiToWcs
 * pb 2007/08/14 underscores for names _Melder_malloc and _Melder_calloc
 * pb 2007/12/05 Melder_wcsequ_firstCharacterCaseInsensitive
 * pb 2009/03/14 counting reallocs moving and in situ
 * pb 2009/07/31 tracing by Melder_debug 34
 * pb 2010/12/28 split into _e and _f versions, and created a rainy-day fund
 * pb 2011/04/05 C++
 * pb 2014/12/17 made everything int64_t
 */

#include "melder.h"
#include <wctype.h>
#include <assert.h>

static int64 totalNumberOfAllocations = 0, totalNumberOfDeallocations = 0, totalAllocationSize = 0,
	totalNumberOfMovingReallocs = 0, totalNumberOfReallocsInSitu = 0;

/*
 * The rainy-day fund.
 *
 * Typically, memory allocation for data is entirely checked by using the normal versions of the allocation routines,
 * which will call Melder_error if they are out of memory.
 * When data allocation is indeed out of memory,
 * the application will present an error message to the user saying that the data could not be created.
 *
 * By contrast, it is not practical to check the allocation of user interface elements,
 * because the application cannot perform correctly if an interface element is missing or incorrect.
 * For such situations, the application will typically use the _f versions of the allocation routines,
 * which, if out of memory, will free a "rainy-day fund" and retry.
 * In this way, the interface element can usually still be allocated;
 * the application will present an error message telling the user to save her work and quit the application.
 * If the user doesn't do that, the application will crash upon the next failing allocation of a _f routine.
 */

#define theRainyDayFund_SIZE  3000000
static char *theRainyDayFund = NULL;

void Melder_alloc_init (void) {
	theRainyDayFund = (char *) malloc (theRainyDayFund_SIZE);   // called at application initialization, so cannot fail
	assert (theRainyDayFund != NULL);
}

void * _Melder_malloc (int64 size) {
	if (size <= 0)
		Melder_throw (U"Can never allocate ", Melder_bigInteger (size), U" bytes.");
	if (sizeof (size_t) < 8 && size > SIZE_MAX)
		Melder_throw (U"Can never allocate ", Melder_bigInteger (size), U" bytes. Use a 64-bit edition of Praat instead?");
	void *result = malloc ((size_t) size);   // guarded cast
	if (result == NULL)
		Melder_throw (U"Out of memory: there is not enough room for another ", Melder_bigInteger (size), U" bytes.");
	if (Melder_debug == 34) { Melder_casual (U"Melder_malloc\t", Melder_pointer (result), U"\t", Melder_bigInteger (size), U"\t1"); }
	totalNumberOfAllocations += 1;
	totalAllocationSize += size;
	return result;
}

void * _Melder_malloc_f (int64 size) {
	if (size <= 0)
		Melder_fatal (U"(Melder_malloc_f:) Can never allocate ", Melder_bigInteger (size), U" bytes.");
	if (sizeof (size_t) < 8 && size > SIZE_MAX)
		Melder_fatal (U"(Melder_malloc_f:) Can never allocate ", Melder_bigInteger (size), U" bytes.");
	void *result = malloc ((size_t) size);
	if (result == NULL) {
		if (theRainyDayFund != NULL) { free (theRainyDayFund); theRainyDayFund = NULL; }
		result = malloc ((size_t) size);
		if (result != NULL) {
			Melder_flushError (U"Praat is very low on memory.\nSave your work and quit Praat.\nIf you don't do that, Praat may crash.");
		} else {
			Melder_fatal (U"Out of memory: there is not enough room for another %s bytes.", Melder_bigInteger (size));
		}
	}
	totalNumberOfAllocations += 1;
	totalAllocationSize += size;
	return result;
}

void _Melder_free (void **ptr) {
	if (*ptr == NULL) return;
	if (Melder_debug == 34) { Melder_casual (U"Melder_free\t", Melder_pointer (*ptr), U"\t?\t?"); }
	free (*ptr);
	*ptr = NULL;
	totalNumberOfDeallocations += 1;
}

void * Melder_realloc (void *ptr, int64 size) {
	if (size <= 0)
		Melder_throw (U"Can never allocate ", Melder_bigInteger (size), U" bytes.");
	if (sizeof (size_t) < 8 && size > SIZE_MAX)
		Melder_throw (U"Can never allocate ", Melder_bigInteger (size), U" bytes. Use a 64-bit edition of Praat instead?");
	void *result = realloc (ptr, (size_t) size);   // will not show in the statistics...
	if (result == NULL)
		Melder_throw (U"Out of memory. Could not extend room to ", Melder_bigInteger (size), U" bytes.");
	if (ptr == NULL) {   // is it like malloc?
		if (Melder_debug == 34) { Melder_casual (U"Melder_realloc\t", Melder_pointer (result), U"\t", Melder_bigInteger (size), U"\t1"); }
		totalNumberOfAllocations += 1;
		totalAllocationSize += size;
	} else if (result != ptr) {   // did realloc do a malloc-and-free?
		totalNumberOfAllocations += 1;
		totalAllocationSize += size;
		totalNumberOfDeallocations += 1;
		totalNumberOfMovingReallocs += 1;
	} else {
		totalNumberOfReallocsInSitu += 1;
	}
	return result;
}

void * Melder_realloc_f (void *ptr, int64 size) {
	void *result;
	if (size <= 0)
		Melder_fatal (U"(Melder_realloc_f:) Can never allocate ", Melder_bigInteger (size), U" bytes.");
	if (sizeof (size_t) < 8 && size > SIZE_MAX)
		Melder_fatal (U"(Melder_realloc_f:) Can never allocate ", Melder_bigInteger (size), U" bytes.");
	result = realloc (ptr, (size_t) size);   // will not show in the statistics...
	if (result == NULL) {
		if (theRainyDayFund != NULL) { free (theRainyDayFund); theRainyDayFund = NULL; }
		result = realloc (ptr, (size_t) size);
		if (result != NULL) {
			Melder_flushError (U"Praat is very low on memory.\nSave your work and quit Praat.\nIf you don't do that, Praat may crash.");
		} else {
			Melder_fatal (U"Out of memory. Could not extend room to ", Melder_bigInteger (size), U" bytes.");
		}
	}
	if (ptr == NULL) {   // is it like malloc?
		totalNumberOfAllocations += 1;
		totalAllocationSize += size;
	} else if (result != ptr) {   // did realloc do a malloc-and-free?
		totalNumberOfAllocations += 1;
		totalAllocationSize += size;
		totalNumberOfDeallocations += 1;
		totalNumberOfMovingReallocs += 1;
	} else {
		totalNumberOfReallocsInSitu += 1;
	}
	return result;
}

void * _Melder_calloc (int64 nelem, int64 elsize) {
	if (nelem <= 0)
		Melder_throw (U"Can never allocate ", Melder_bigInteger (nelem), U" elements.");
	if (elsize <= 0)
		Melder_throw (U"Can never allocate elements whose size is ", Melder_bigInteger (elsize), U" bytes.");
	if ((uint64_t) nelem > SIZE_MAX / (uint64_t) elsize)   // guarded casts to unsigned
		Melder_throw (U"Can never allocate ", Melder_bigInteger (nelem), U" elements whose sizes are ", Melder_bigInteger (elsize), U" bytes each.",
			sizeof (size_t) < 8 ? U" Use a 64-bit edition of Praat instead?" : NULL);
	void *result = calloc ((size_t) nelem, (size_t) elsize);
	if (result == NULL)
		Melder_throw (U"Out of memory: there is not enough room for ", Melder_bigInteger (nelem), U" more elements whose sizes are ", elsize, U" bytes each.");
	if (Melder_debug == 34) { Melder_casual (U"Melder_calloc\t", Melder_pointer (result), U"\t", Melder_bigInteger (nelem), U"\t", Melder_bigInteger (elsize)); }
	totalNumberOfAllocations += 1;
	totalAllocationSize += nelem * elsize;
	return result;
}

void * _Melder_calloc_f (int64 nelem, int64 elsize) {
	if (nelem <= 0)
		Melder_fatal (U"(Melder_calloc_f:) Can never allocate ", Melder_bigInteger (nelem), U" elements.");
	if (elsize <= 0)
		Melder_fatal (U"(Melder_calloc_f:) Can never allocate elements whose size is ", Melder_bigInteger (elsize), U" bytes.");
	if ((uint64_t) nelem > SIZE_MAX / (uint64_t) elsize)
		Melder_fatal (U"(Melder_calloc_f:) Can never allocate ", Melder_bigInteger (nelem), U" elements whose sizes are ", Melder_bigInteger (elsize), U" bytes each.");
	void *result = calloc ((size_t) nelem, (size_t) elsize);
	if (result == NULL) {
		if (theRainyDayFund != NULL) { free (theRainyDayFund); theRainyDayFund = NULL; }
		result = calloc ((size_t) nelem, (size_t) elsize);
		if (result != NULL) {
			Melder_flushError (U"Praat is very low on memory.\nSave your work and quit Praat.\nIf you don't do that, Praat may crash.");
		} else {
			Melder_fatal (U"Out of memory: there is not enough room for ", Melder_bigInteger (nelem),
				U" more elements whose sizes are ", Melder_bigInteger (elsize), U" bytes each.");
		}
	}
	totalNumberOfAllocations += 1;
	totalAllocationSize += nelem * elsize;
	return result;
}

char * Melder_strdup (const char *string) {
	if (! string) return NULL;
	int64 size = (int64) strlen (string) + 1;
	if (sizeof (size_t) < 8 && size > SIZE_MAX)
		Melder_throw (U"Can never allocate ", Melder_bigInteger (size), U" bytes. Use a 64-bit edition of Praat instead?");
	char *result = (char *) malloc ((size_t) size);
	if (result == NULL)
		Melder_throw (U"Out of memory: there is not enough room to duplicate a text of ", Melder_bigInteger (size - 1), U" characters.");
	strcpy (result, string);
	if (Melder_debug == 34) { Melder_casual (U"Melder_strdup\t", Melder_pointer (result), U"\t", Melder_bigInteger (size), U"\t1"); }
	totalNumberOfAllocations += 1;
	totalAllocationSize += size;
	return result;
}

char * Melder_strdup_f (const char *string) {
	if (! string) return NULL;
	int64 size = (int64) strlen (string) + 1;
	if (sizeof (size_t) < 8 && size > SIZE_MAX)
		Melder_fatal (U"(Melder_strdup_f:) Can never allocate ", Melder_bigInteger (size), U" bytes.");
	char *result = (char *) malloc ((size_t) size);
	if (result == NULL) {
		if (theRainyDayFund != NULL) { free (theRainyDayFund); theRainyDayFund = NULL; }
		result = (char *) malloc ((size_t) size * sizeof (char));
		if (result != NULL) {
			Melder_flushError (U"Praat is very low on memory.\nSave your work and quit Praat.\nIf you don't do that, Praat may crash.");
		} else {
			Melder_fatal (U"Out of memory: there is not enough room to duplicate a text of ", Melder_bigInteger (size - 1), U" characters.");
		}
	}
	strcpy (result, string);
	totalNumberOfAllocations += 1;
	totalAllocationSize += size;
	return result;
}

char32 * Melder_dup (const char32 *string /* cattable */) {
	if (! string) return NULL;
	int64 size = (int64) str32len (string) + 1;   // guaranteed to be positive
	if (sizeof (size_t) < 8 && size > SIZE_MAX / sizeof (char32))
		Melder_throw (U"Can never allocate ", Melder_bigInteger (size), U" characters. Use a 64-bit edition of Praat instead?");
	char32 *result = (char32 *) malloc ((size_t) size * sizeof (char32));   // guarded conversion
	if (result == NULL)
		Melder_throw (U"Out of memory: there is not enough room to duplicate a text of ", Melder_bigInteger (size - 1), U" characters.");
	str32cpy (result, string);
	if (Melder_debug == 34) { Melder_casual (U"Melder_dup\t", Melder_pointer (result), U"\t", Melder_bigInteger (size), U"\t4"); }
	totalNumberOfAllocations += 1;
	totalAllocationSize += size * (int64) sizeof (char32);
	return result;
}

char32 * Melder_dup_f (const char32 *string /* cattable */) {
	if (! string) return NULL;
	int64 size = (int64) str32len (string) + 1;
	if (sizeof (size_t) < 8 && size > SIZE_MAX / sizeof (char32))
		Melder_fatal (U"(Melder_dup_f:) Can never allocate ", Melder_bigInteger (size), U" characters.");
	char32 *result = (char32 *) malloc ((size_t) size * sizeof (char32));
	if (result == NULL) {
		if (theRainyDayFund != NULL) { free (theRainyDayFund); theRainyDayFund = NULL; }
		result = (char32 *) malloc ((size_t) size * sizeof (char32));
		if (result != NULL) {
			Melder_flushError (U"Praat is very low on memory.\nSave your work and quit Praat.\nIf you don't do that, Praat may crash.");
		} else {
			Melder_fatal (U"Out of memory: there is not enough room to duplicate a text of ", Melder_bigInteger (size - 1), U" characters.");
		}
	}
	str32cpy (result, string);
	totalNumberOfAllocations += 1;
	totalAllocationSize += size * (int64) sizeof (char32);
	return result;
}

int64 Melder_allocationCount (void) {
	return totalNumberOfAllocations;
}

int64 Melder_deallocationCount (void) {
	return totalNumberOfDeallocations;
}

int64 Melder_allocationSize (void) {
	return totalAllocationSize;
}

int64 Melder_reallocationsInSituCount (void) {
	return totalNumberOfReallocsInSitu;
}

int64 Melder_movingReallocationsCount (void) {
	return totalNumberOfMovingReallocs;
}

int Melder_cmp (const char32 *string1, const char32 *string2) {
	if (string1 == NULL) string1 = U"";
	if (string2 == NULL) string2 = U"";
	return str32cmp (string1, string2);
}

int Melder_ncmp (const char32 *string1, const char32 *string2, int64_t n) {
	if (string1 == NULL) string1 = U"";
	if (string2 == NULL) string2 = U"";
	return str32ncmp (string1, string2, n);
}

bool Melder_str32equ_firstCharacterCaseInsensitive (const char32 *string1, const char32 *string2) {
	if (string1 == NULL) string1 = U"";
	if (string2 == NULL) string2 = U"";
	if (*string1 == U'\0') return *string2 == U'\0';
	if (*string1 == *string2)
		return ! str32cmp (string1 + 1, string2 + 1);
	if (sizeof (wchar_t) == 2) {
		if (*string1 > 65536 || *string2 > 65536)
			return false;
		if (towlower ((char16) *string1) != towlower ((char16) *string2)) return false;
	} else {
		if (towlower ((int32) *string1) != towlower ((int32) *string2)) return false;
	}
	return ! str32cmp (string1 + 1, string2 + 1);
}

char32 * Melder_tok (char32 *string, const char32 *delimiter) {
	const char32 *spanp;
	char32 c, sc;
	char32 *tok;
	static char32 *last;

	if (string == NULL && (string = last) == NULL)
		return (NULL);

cont:
	c = * string ++;
	for (spanp = delimiter; (sc = * spanp ++) != U'\0';) {
		if (c == sc)
			goto cont;
	}

	if (c == U'\0') {
		last = NULL;
		return NULL;
	}
	tok = string - 1;
	for (;;) {
		c = * string ++;
		spanp = delimiter;
		do {
			if ((sc = * spanp ++) == c) {
				if (c == U'\0')
					string = NULL;
				else
					string [-1] = U'\0';
				last = string;
				return tok;
			}
		} while (sc != U'\0');
	}
}

/* End of file melder_alloc.cpp */
