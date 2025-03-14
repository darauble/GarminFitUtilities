# Garmin FIT Utilities

This is a project of a few utilities to tinker with Garmin FIT files. There are a few available online utilities and one quite good commercial one, but they don't provide all I need for my own liking.

At the moment utilities are command-line only and tested only on Linux. I keep the code as generic as possible using C++ 20 standard library and nothing from POSIX.

Code is not very well organized and consistent. In one utility I use CLI arguments parser, in others I use raw `argc` and `argv`. I use Garmin FIT SDK event-based parser to retrieve coordinates, but for other needs I implemented my own binary FIT parser, which works with bytes directly at their place to modify the files if needed (and is very fast compared to FIT SDK).

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

My original intent was to find out how many times I passed by one of the two cycling counters that I know of and ride by quite often. Then I thought it would be fun to see how many times I crossed one or another bridge. So here's why I created this utility.

An example command:

`garmin-points-visited -s cycling -l 54.9284880827859823 -o 23.7503685882586737 -i "~/SPORTAS/eksportuoti-workoutai/2024"`

The final output (not showin the debug output here):

```
===== Parsing Summary =====
Total files:    832
Parsed files:   821
Filtered files: 94
Total visits:   13
===========================
```

So in the year 2024 I visited one counter just 13 times! Obviously I did more gravel and forest riding than the usual road. And only 94 rides... well, but I have to run, hike/ruck, swim, do lifting and yoga too... time is limited for decathletes :-D

## Rename Files

`garmin-rename-files`

I usually backup FIT files from my watches. They are named in the fashion of date-time. However, there was some busy time and I did not do it for some months. All files, of course, are uploaded to Connect, but when downloaded, they have a name of `<long-id>_ACTIVITY.fit`. I don't like that, I prefer date in the name.

I requested all my files download from Garmin and in a couple of days received a link to a huge archive. All files there, unfortunately, were named as mentioned.

So I developed a utility, which scans a given directory for files, reads Fild ID message and uses this timestamp to rename the file with a date-time format. Note, this is not the _activity start_ date, but file creation date. It is a bit different, as file is created _after_ the activity has finished. But this is in line with the file naming on the device.

## Editor/Analyzer

`garmin-edit`

It started as a part of Garmin badges pursuit. There are some badges that are tied to particular watches, most notably the Legacy Hero series - four different watches. If the watch number is changed in the workout and it's re-uploaded to the Connect, one can receive badges related to the particular watch (like the "Feel the Force" of the Rey watch).

Simply hex-editing FIT file wouldn't work, because at the end of the file there's CRC. Connect _does not accept_ FIT files with badly formed CRC.

One workaround is to export the activity as TCX file, which is a XML and can be edited by hand. But when re-uploaded, it misses some statistics and measures, so it is not an exact representation of the FIT metrics. This might be important for a dedicated athlete. So it would be best to modify just the product number in the original file and recalculate the CRC.

Initially I tried to decode the file with Garmin SDK and re-assemble it. Did not work well, SDK did not catch all the messages, including some developer messages and fields. So I started with the idea to learn to decode the FIT files and create some structure, mapping the byte blob.

FIT file internals are very well detailed by Garmin in their [FIT Protocol description](https://developer.garmin.com/fit/protocol/).

Product replacement was the first more serious project. It boosted the idea to create something of my own "SDK", have fun with classes and inheritance.

### "SDK"

`binary-mapper` is the utility, which reads the FIT file, creates two vectors of message definitions and messages themselves. Everything is based on the offset in the FIT file, i.e. "this field is at the byte no. XXX". It fills in the structure of definitions and actual messages and leaves them in the same order, as in the file. This is in contrary to C++ Garmin SDK, which is a streaming parser, similar to SAX (as opposed to DOM, which can be explored instead of run-through).

`binary-scanner` is the part of the code, which takes mapped file an can "scan" through it, calling the `record` method for each message, mapped with its definition. This can be inherited for specific purposes - like the `product-scanner` to find the product descriptions in the FIT file.

### Using the Editor

It is a command line utility. I don't reject the idea of adding GUI later, but that's a low priority.

Utility has the hierarchy of action and command. E.g. to view current device ("product" in Garmin's parlance) of the file use:

`garmin-edit show product <file name>`

**show** is the action, the **product** is the command. To replace the product use:

`garmin-edit replace product <old product ID> <new product ID> <old file> <new file>`

Product IDs are numbers, assigned to a particular device. E.g. Garmin Forerunner 945 has the ID of 3113, whereeas Rey watch has the ID of 3498.

Investigate `fit_profile.hpp` in the Garmin FIT SDK, all products are there. When new products are released, so is the new version of the SDK :-)

`garmin-edit show help` should help you a bit ;-)

One more command - `timestamp`. It can be used to adjust the time of the activity. Usually it is not a very useful thing, but sometimes I track non-sports related activities with the chronometer (and chronometer can be saved as an activity!). Sometimes I forget to start the chronometer at the right time. This is not a very big problem: Connect allows editing the start of the activity and its length, but that _does not affect the original file_. I just wanted to have possibility to do that. I love the idea of owning my data.

The third command `message` shows the file contents in the readable table format:

```
====  Message #0 (file_id)  ====
+-----------------+----------------+------------+----------------+-----------+----------+--------+
| serial_number 3 | time_created 4 |          7 | manufacturer 1 | product 2 | number 5 | type 0 |
+-----------------+----------------+------------+----------------+-----------+----------+--------+
|      XXXXXXXXXX |     1110291631 | 4294967295 |              1 |      XXXX |    65535 |      4 |
+-----------------+----------------+------------+----------------+-----------+----------+--------+

====  Message #49 (file_creator)  ====
+----------------------+--------------------+--------------------+-----+-----+
|                    2 | software_version 0 | hardware_version 1 |   3 |   4 |
+----------------------+--------------------+--------------------+-----+-----+
|                      |               XXXX |                255 | 255 | 255 |
+----------------------+--------------------+--------------------+-----+-----+

====  Message #34 (activity)  ====
+---------------+--------------------+-------------------+------------------------------------------------------------------+----------------+--------+---------+--------------+---------------+-----+
| timestamp 253 | total_timer_time 0 | local_timestamp 5 |                                                                8 | num_sessions 1 | type 2 | event 3 | event_type 4 | event_group 6 |   7 |
+---------------+--------------------+-------------------+------------------------------------------------------------------+----------------+--------+---------+--------------+---------------+-----+
|    1110291631 |            2013389 |        1110298831 |                                                                  |              1 |      0 |      26 |            1 |           255 | 255 |
+---------------+--------------------+-------------------+------------------------------------------------------------------+----------------+--------+---------+--------------+---------------+-----+

====  Message #18 (session)  ====
+---------------+--------------+----------------------+-----------------------+----------------------+--------------------+------------------+-----------------+-------------+-------------+-------------+-------------+---------------------+----------------------+---------------------+---------------+----------------------+------------+----------------------------------------------------------------------------------------------------------------------------------+-------------------+--------------------------+-------------------------------+---------------------------+--------------------------------+------------------------+------------------------+------------------------+------------------------+------------+------------------------+----------------+--------------+------------+------------+-------------------+-------------------+--------------+--------------+--------------+--------------+-----------------+------------------+--------------------+-------------+----------------+---------------------+--------------------------+---------------------+-----------------------+------------------------+----------------+--------------------+-----------------------+-------+-------+-----------------------------+----------------------------+--------------------+-------+-------+-------+-----------------+------------------------+-----------------------------+---------------------+-------+-------+-------+-----------------------------------+-----------------------------------+-------+-------+-------+-----------------------------------+-------+-------+-------+---------+--------------+---------+-------------+-------------------+-------------------+----------------+----------------+--------------------------+----------------+------------+----------------+---------------------+--------------------+--------------------+-----+---------------------------+---------------------------+----------------------------+-----------------------------------+------------------------------------+-------------------------------+--------------------------------+-----------------------------------+------+------------------+-------------------+--------------------------+--------------------------+-------------------------------------+------+--------------------------+--------------------------+--------------------------+---------------------+------+------+------+------------------+-----------------+--------------+----------------+--------------+---------------+------+------+------+------+------+------+
| timestamp 253 | start_time 2 | start_position_lat 3 | start_position_long 4 | total_elapsed_time 7 | total_timer_time 8 | total_distance 9 | total_cycles 10 |  nec_lat 29 | nec_long 30 |  swc_lat 31 | swc_long 32 | end_position_lat 38 | end_position_long 39 | avg_stroke_count 41 | total_work 48 | total_moving_time 59 |         78 |                                                                                                           sport_profile_name 110 | time_standing 112 | avg_left_power_phase 116 | avg_left_power_phase_peak 117 | avg_right_power_phase 118 | avg_right_power_phase_peak 119 | avg_power_position 120 | max_power_position 121 | enhanced_avg_speed 124 | enhanced_max_speed 125 |        152 | training_load_peak 168 | total_grit 181 | avg_flow 187 |        211 |        218 | message_index 254 | total_calories 11 | avg_speed 14 | max_speed 15 | avg_power 20 | max_power 21 | total_ascent 22 | total_descent 23 | first_lap_index 25 | num_laps 26 | num_lengths 33 | normalized_power 34 | training_stress_score 35 | intensity_factor 36 | left_right_balance 37 | avg_stroke_distance 42 | pool_length 44 | threshold_power 45 | num_active_lengths 47 |    79 |    80 | avg_vertical_oscillation 89 | avg_stance_time_percent 90 | avg_stance_time 91 |   106 |   107 |   108 | stand_count 113 | avg_vertical_ratio 132 | avg_stance_time_balance 133 | avg_step_length 134 |   151 |   157 |   158 | enhanced_avg_respiration_rate 169 | enhanced_max_respiration_rate 170 |   177 |   178 |   179 | enhanced_min_respiration_rate 180 |   189 |   190 |   196 | event 0 | event_type 1 | sport 5 | sub_sport 6 | avg_heart_rate 16 | max_heart_rate 17 | avg_cadence 18 | max_cadence 19 | total_training_effect 24 | event_group 27 | trigger 28 | swim_stroke 43 | pool_length_unit 46 | avg_temperature 57 | max_temperature 58 |  81 | avg_fractional_cadence 92 | max_fractional_cadence 93 | total_fractional_cycles 94 | avg_left_torque_effectiveness 101 | avg_right_torque_effectiveness 102 | avg_left_pedal_smoothness 103 | avg_right_pedal_smoothness 104 | avg_combined_pedal_smoothness 105 |  109 | avg_left_pco 114 | avg_right_pco 115 | avg_cadence_position 122 | max_cadence_position 123 | total_anaerobic_training_effect 137 |  138 | avg_respiration_rate 147 | max_respiration_rate 148 | min_respiration_rate 149 | min_temperature 150 |  184 |  185 |  188 | workout_feel 192 | workout_rpe 193 | avg_spo2 194 | avg_stress 195 | sdrr_hrv 197 | rmssd_hrv 198 |  201 |  202 |  205 |  206 |  207 |  212 |
+---------------+--------------+----------------------+-----------------------+----------------------+--------------------+------------------+-----------------+-------------+-------------+-------------+-------------+---------------------+----------------------+---------------------+---------------+----------------------+------------+----------------------------------------------------------------------------------------------------------------------------------+-------------------+--------------------------+-------------------------------+---------------------------+--------------------------------+------------------------+------------------------+------------------------+------------------------+------------+------------------------+----------------+--------------+------------+------------+-------------------+-------------------+--------------+--------------+--------------+--------------+-----------------+------------------+--------------------+-------------+----------------+---------------------+--------------------------+---------------------+-----------------------+------------------------+----------------+--------------------+-----------------------+-------+-------+-----------------------------+----------------------------+--------------------+-------+-------+-------+-----------------+------------------------+-----------------------------+---------------------+-------+-------+-------+-----------------------------------+-----------------------------------+-------+-------+-------+-----------------------------------+-------+-------+-------+---------+--------------+---------+-------------+-------------------+-------------------+----------------+----------------+--------------------------+----------------+------------+----------------+---------------------+--------------------+--------------------+-----+---------------------------+---------------------------+----------------------------+-----------------------------------+------------------------------------+-------------------------------+--------------------------------+-----------------------------------+------+------------------+-------------------+--------------------------+--------------------------+-------------------------------------+------+--------------------------+--------------------------+--------------------------+---------------------+------+------+------+------------------+-----------------+--------------+----------------+--------------+---------------+------+------+------+------+------+------+
|    1110291631 |   1110291631 |           2147483647 |            2147483647 |              2013389 |            2013389 |                0 |             320 |  2147483647 |  2147483647 |  2147483647 |  2147483647 |          2147483647 |           2147483647 |          4294967295 |    4294967295 |           4294967295 |    1211270 |                                                                                                                             Jėga |        4294967295 |                      255 |                           255 |                       255 |                            255 |                  65535 |                  65535 |                      0 |             4294967295 |      86800 |                3942653 |           -nan |         -nan | 4294967295 | 4294967295 |                 0 |               247 |        65535 |        65535 |        65535 |        65535 |           65535 |            65535 |                  0 |          36 |          65535 |               65535 |                    65535 |               65535 |                 65535 |                  65535 |          65535 |              65535 |                 65535 | 65535 | 65535 |                       65535 |                      65535 |              65535 |     0 | 65535 | 65535 |           65535 |                  65535 |                       65535 |               65535 |    18 | 65535 | 65535 |                              2578 |                              3876 | 65535 |   419 | 65535 |                              1711 | 65535 | 65535 |    45 |       8 |            1 |      10 |          20 |               121 |               170 |            255 |            255 |                       22 |            255 |          0 |            255 |                 255 |                 29 |                 30 |   0 |                       255 |                       255 |                        255 |                               255 |                                255 |                           255 |                            255 |                               255 |  255 |              127 |               127 |                      255 |                      255 |                                  20 |    1 |                      255 |                      255 |                      255 |                  28 |    0 |  255 |    6 |              255 |             255 |          255 |            255 |          255 |           255 |  255 |  255 |  255 |  255 |  255 |  255 |
+---------------+--------------+----------------------+-----------------------+----------------------+--------------------+------------------+-----------------+-------------+-------------+-------------+-------------+---------------------+----------------------+---------------------+---------------+----------------------+------------+----------------------------------------------------------------------------------------------------------------------------------+-------------------+--------------------------+-------------------------------+---------------------------+--------------------------------+------------------------+------------------------+------------------------+------------------------+------------+------------------------+----------------+--------------+------------+------------+-------------------+-------------------+--------------+--------------+--------------+--------------+-----------------+------------------+--------------------+-------------+----------------+---------------------+--------------------------+---------------------+-----------------------+------------------------+----------------+--------------------+-----------------------+-------+-------+-----------------------------+----------------------------+--------------------+-------+-------+-------+-----------------+------------------------+-----------------------------+---------------------+-------+-------+-------+-----------------------------------+-----------------------------------+-------+-------+-------+-----------------------------------+-------+-------+-------+---------+--------------+---------+-------------+-------------------+-------------------+----------------+----------------+--------------------------+----------------+------------+----------------+---------------------+--------------------+--------------------+-----+---------------------------+---------------------------+----------------------------+-----------------------------------+------------------------------------+-------------------------------+--------------------------------+-----------------------------------+------+------------------+-------------------+--------------------------+--------------------------+-------------------------------------+------+--------------------------+--------------------------+--------------------------+---------------------+------+------+------+------------------+-----------------+--------------+----------------+--------------+---------------+------+------+------+------+------+------+
```

I have a few plans for the `message` command:
* Show offset of each data field (optional)
* Filter desired message(s) by number/name
* Filter desired field(s) in the message(s) by number/name

# Building

I'm not going to release the binaries, so compile at your own leisure. There are no dependencies except for the [FIT SDK](https://developer.garmin.com/fit/download/). Download it and place at the same directory level, as this project, i.e. `GarminFitUtilities`

So the directory structure should be as follows (but adjust for the SDK version):

```
.
├── FitSDKRelease_21.158.00
└── GarminFitUtilities
```

If you have different SDK version, edit the top `CMakeLists.txt` file in the `GarminFitUtilities`.

If you don't require all three (currently) utilities, there are options in the top `CMakeLists.txt` file to turn their compilation off.

Then:
* `cd GarminFitUtilities`
* `mkdir build`
* `cmake ..`
* `make`

*NOTE:* All C++ FIT SDK is compiled as a shared library. Resulting utilities will have it's path compiled in them (`set(CMAKE_SKIP_RPATH TRUE)` is commented out).

Currently there's no `make install` target, but _maybe_ I'll add later.

Have fun!
