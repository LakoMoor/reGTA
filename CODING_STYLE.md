# Coding style

I started writing in [Plan 9 style](http://man.cat-v.org/plan_9/6/style),
but realize that this is not the most popular style, so I'm willing to compromise.
Try not to deviate too much so the code will look similar across the whole project.

To give examples, these two styles (or anything in between) are fine:

```
type
functionname(args)
{
	if(a == b){
		s1;
		s2;
	}else{
		s3;
		s4;
	}
	if(x != y)
		s5;
}

type functionname(args)
{
	if (a == b) {
		s1;
		s2;
	} else {
		s3;
		s4;
	}
	if (x != y)
		s5;
}
```

This one (or anything more extreme) is heavily discouraged:

```
type functionname ( args )
{
  if ( a == b )
  {
    s1;
    s2;
  }
  else
  {
    s3;
    s4;
  }
  if ( x != y )
  {
    s5;
  }
}
```

i.e. 

* Put the brace on the same line as control statements

* Put the brace on the next line after function definitions and structs/classes

* Put an `else` on the same line with the braces

* Don't put braces around single statements

* Put the function return type on a separate line

* Indent with TABS

As for the less cosmetic choices, here are some guidelines how the code should look:

* Don't use magic numbers where the original source code would have had an enum or similar.
Even if you don't know the exact meaning it's better to call something `FOOBAR_TYPE_4` than just `4`,
since `4` will be used in other places and you can't easily see where else the enum value is used.

* Don't just copy paste code from IDA, make it look nice

* Use the right types. In particular:

    * don't use types like `__int16`, we have `int16` for that

    * don't use `unsigned`, we have typedefs for that

    * don't use `char` for anything but actual characters, use `int8`, `uint8` or `bool`

    * don't even think about using win32 types (`BYTE`, `WORD`, &c.) unless you're writing win32 specific code

    * declare pointers like `int *ptr;`, not `int* ptr;`

* As for variable names, the original gta source code was not written in a uniform style,
but here are some observations:

    * many variables employ a form of hungarian notation, i.e.:

    * `m_` may be used for class member variables (mostly those that are considered private)

    * `ms_` for (mostly private) static members

    * `f` is a float, `i` or `n` is an integer, `b` is a boolean, `a` is an array

    * do *not* use `dw` for `DWORD` or so, we're not programming win32

* Generally, try to make the code look as if R* could have written it
