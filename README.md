# Garmin FIT Utilities

This is a project of a few utilities to tinker with Garmin FIT files. There are a few available online utilities and one quite good commercial one, but they don't provide all I need for my own liking.

At the moment utilities are command-line only and tested only on Linux. I keep the code as generic as possible using C++ 20 standard library and nothing from POSIX.

Code is not very well organized and consistent. In one utility I use CLI arguments parser, in others I use raw `argc` and `argv`. I use Garmin SDK event-based parser to retrieve coordinates, but for other needs I implemented my own binary FIT parser, which works with bytes directly at their place to modify the files if needed (and is very fast compared to SDK's).

This is a part of my wish to refresh Modern C++ knowledge, so again: not very well organized and some class constructions duplicate the code and are superfluous - but some help to keep the code divided better.

Utilities also print a lot of output. Simply because that helps me find some bugs and look for additional details if I care. Everything for my own fun.

There are three utilities at the moment.

## Points Visited

`garmin-points-visited`

This utility takes the coordinates from input and scans given directory for `*.fit` files (or just takes one file). It then scans the coordinates of the file track and checks if track visited the given point.

By default a bounding box is caluculated around the given point, a square of 20x20 meters. Another bounding box is created between subsequent pairs of points in the FIT track segment and then there's a check if bounding boxes overlap.

*For those, who are interested.* It is _much easier_ to check if bounding boxes _don't overlap!_ I.e. if one is above or below another, or to the left or right of another. So in the code I check if boxes _don't overlap_ and then invert that check to see if they overlap :-D That's some fun of the algorithm.

At the end utility prints a simple summary: how many files were found, how many of them went past given point etc.

*Note:* it is possible to filter out sports. E.g. scan only cycling, running, hiking, walking or even swimming.

My original intent was to find out how many times I passed by one of the two cycling counters that I know of and ride by quite often. Then I thought it would be fun to see how many times I crossed one or another bridge. So here's why I made this utility.

## Rename Files

`garmin-rename-files`

I usually backup FIT files from my watches. They are named in the fashion of date-time. However, there was some busy time and I did not do it for some months. All files, of course, are uploaded to Connect, but when downloaded, they have a name of `<long-id>_ACTIVITY.fit`. I don't like that, I prefer date in the name.

I requested all my files download from Garmin and in a couple of days received a link to a huge archive. All files there, unfortunately, were named as mentioned.

So I developed a utility, which scans a given directory for files, reads Fild ID message and uses this timestamp to rename the file with a date-time format. Note, this is not the _activity start_ date, but file creation date. It is a bit different, as file is created _after_ the activity has finished. But this is in line with the file naming on the device.

## Editor/Analyzer

`garmin-edit`

