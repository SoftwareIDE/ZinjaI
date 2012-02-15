 Instructions for writing new translations
===========================================

In order to have a new language completelly available, ZinjaI will need this:

1) a .pre file with all gui strings translated (for instance: lang/english.pre)
2) some html quickhelp files (for instance: guihelp/pref_help_spanish.html)
3) an html welcome panel (for instance: imgs/welcome_panel_english.html)
4) a tips file (for instance: quickhelp/tips_english.html)
5) some elements in c string's array in runner.cpp

You should not need to know anything about programming in order to do the first 4 steps. The last one is also very simple, but involves a source file.
If you make a new translation, you can send it to zaskar_84@yahoo.com.ar, and this will be included in ZinjaI's package for next release. If people start doing it, I'll add some extra file for credits, so tell send me your name and also the name of others who have helped you. If you have any question about how to do this, or what does a string mean, you can write to the same email address.

1) .pre file
============
For any new language, there must be a .pre file in lang folder. The filename will be the language name plus .pre extension. For convinience, the filename will contain only lowercarse characters, and no especial symbols. The name will be the english word for that language. This file must not be writen directly. The format in this file is the following:
* first line containing "ZINJAI_LANGUAGE_FILE"
* second line containing the file name (without extension)
* 2xN lines containing pairs of keyword+translation
* last line containing just "END"
The first two lines could be in fact anything, actually they are just ignored, but they may be used in future releases for adding some extra information. The next lines are translations, and keywords must follow and exact order. This order will vary between releases (new keywords may be added in any place). So, to make this part easy, there is a tool in lang/tools called mxLangTool. This tool must be run from ZinjaI's main directory. It will use .src files. Theese files are very similar to pre files, but they have less restrictions: the keywords could be in any order, not all keyword must be present, there no two header lines. The tool will let you choose a .src language file to edit and a .src language file to use as reference (to read the strings you must translate). When you save file in that tool, it will create the pre file with sorted translations and will fill the missing ones with the texts from the reference language.
So, what you must do is:
1) Create a new file with 1 line that contain "END" and save it in lang folder with your language name plus .src extension.
2) Load mxLangTool from ZinjaI's folder and choose your new language as language for edition.
3) Translate the strings with that tool and click "Save"
Then, you will see the new language available in ZinjaI, but you'll see also some errors if you try to use it. There will be still missing some html files.

2) html quickhelp files
=======================
These files contain the little help tabs that you see in some dialogs (preferences, project's compiling and running settings, new file wizard,...). Look for files from other language to see who they are. All of them will be in guihelp folder, and their names will end with an underscore plus the language name (for example, for preference dialog in english, it is prefs_help_english.html). You must copy them to the righ filename for your language (the language name must be the one from .pre filename).

3) html welcome panel
=====================
The Welcome Panel's text is stored in a html file in imgs folder. It is called welcome_panel_ plus the language name. This is the default one, but a skin could have its own (following the same naming convention). You will see in this html some especial keywords with sign $. Theese represent the dinamyc content.

4) tips file
============
Tip's file is in quickhelp folder. It's name is tips_ plus the language name. It does not have any extension. It is a plain text file with the following rules:
1) first line contains the message you see the first time you run ZinjaI
2) second line contains the message you see when you have just upgraded ZinjaI
3) third line contains a special tip that will only be shown in tips dialog (but won't in welcome panel)
4) the rest of the line contains other tips in any order
Don't worry about the second line, because it changes in every release, and you don't know what to say for the next one.

5) runner.cpp
=============
The runner is the program that wraps your program. It keeps open a terminal for debugging sessions, wait for a key after execution, set the correct working directory and shows you the exit code when your program finished. Translations for this one are harcoded because. The source file is runner.cpp. You will find there four c string's arrays with the sentences to translate. If you want to add a new language here you must add an item to each array and set the appropiate lang_idx in the first program while, where it parses arguments. If you don't know how to do it, you can send me the strings translated and I'll add them for future releases.
