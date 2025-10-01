# Garmin FIT Utilities

This is a project of a few utilities to tinker with Garmin FIT files. There are a few available online utilities and one quite good commercial one, but they don't provide all I need for my own liking.

At the moment utilities are command-line only and tested only on Linux. I keep the code as generic as possible using C++ 20 standard library and nothing from POSIX.

Code is not very well organized and consistent. In one utility I use CLI arguments parser, in others I use raw `argc` and `argv`. I also originally used Garmin FIT SDK event-based parser to retrieve coordinates, but for other needs I implemented my own binary FIT parser, which works with bytes directly at their place to modify the files if needed (and is very fast compared to FIT SDK). So then I migrated to using my own mappers/scanners framework. All is work in progress :-)

This is a part of my wish to refresh Modern C++ knowledge, so again: not very well organized and some class constructions duplicate the code and are superfluous - but some help to keep the code divided better.

Utilities also print a lot of output. Simply because that helps me find some bugs and look for additional details if I care. Everything for my own fun.

There are three shell utilities and one GUI application uniting them at the moment.

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

I initially used FIT SDK to parse FIT files and retrieve coordinates. This utility was the first to implement, so I thought using SDK would be best. _WRONG!_ I was surprised at how slow it was :-|

After implementing my own "SDK" and a framework I like myself I decided to migrate coordinates reading using my own semi-raw mapper/scanner (described below). The code is not very elegant, I need way more `if` statements and deeper knowledge of the FIT structure, but boy it runs fast!

Example comparison of how much it takes to parse some FIT files (in seconds and _including_ reading from disk):

| File                    | Points | FIT SDK     | Mapper/Scanner |
| ----------------------- | ------ | ----------- | -------------- |
| 2025-03-07-05-12-16.fit | 2144   | 0.033300131 | 0.000976237    |
| 2025-03-08-05-59-28.fit | 2144   | 0.033640553 | 0.000485185    |
| 2025-03-12-05-05-20.fit | 2003   | 0.033037443 | 0.00157116     |
| 2025-03-13-11-04-10.fit | 1429   | 0.021947243 | 0.001335546    |
| 2025-03-18-06-26-06.fit | 796    | 0.010872027 | 0.000289233    |
| 2025-03-21-05-48-57.fit | 1754   | 0.025102158 | 0.000687751    |

And yes, I use various dynamic C++ structures, like vectors, not C type malloc/realloc! Those potentially could be even faster if used correctly.

Parsing all my 2024 year (as mentioned above) with FIT SDK took 10 seconds. Using my own parser - _under a second_. That's a _green_ programming for you (less time, less cycles, less energy wasted).

I like being a low level coder (in my professional life).

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

A helper script is added, called `garmin-edit-completion.sh`. Source it to have `garmin-edit` subcommand completion in the shell.

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

If there are any developer fields (like Stryd fields below), they are displayed with an asterisk at the beginning:

```
====  Message #20 (record)  ====
+---------------------+----------------+-----------------+---------------------------------+----------------------+---------------------------------+---------------------------------+------------+---------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+--------------+-----------+----------------+------------------+---------------------------------+------+------+------+------+------+------+----------+---------------+-------------------------+--------------+------------+----------------+-------------------------+
|       timestamp 253 | position_lat 0 | position_long 1 |                      distance 5 | accumulated_power 29 |               enhanced_speed 73 |            enhanced_altitude 78 |        140 | power 7 |         vertical_oscillation 39 |          stance_time_percent 40 |                  stance_time 41 |               vertical_ratio 83 |          stance_time_balance 84 |                  step_length 85 |               cycle_length16 87 |   enhanced_respiration_rate 108 | heart_rate 3 | cadence 4 | temperature 13 | activity_type 42 |           fractional_cadence 53 |  107 |  134 |  137 |  138 |  143 |  144 | *Power 0 | *Form Power 8 | *Leg Spring Stiffness 9 | *Elevation 7 | *Cadence 2 | *Ground Time 3 | *Vertical Oscillation 4 |
+---------------------+----------------+-----------------+---------------------------------+----------------------+---------------------------------+---------------------------------+------------+---------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+--------------+-----------+----------------+------------------+---------------------------------+------+------+------+------+------+------+----------+---------------+-------------------------+--------------+------------+----------------+-------------------------+
| 2025-03-24 06:09:52 |      654277831 |       284319777 |                           15.48 |                 1708 |                           2.547 |                            85.6 |       2553 |     269 |                            82.8 |                           65535 |                             272 |                            9.21 |                           65535 |                             899 |                               0 |                           16.53 |           76 |        85 |             27 |                1 |                               0 |    1 |  255 |   97 |   97 |  100 |   76 |       16 |           199 |                     199 |          199 |        199 |            199 |                     199 |
+---------------------+----------------+-----------------+---------------------------------+----------------------+---------------------------------+---------------------------------+------------+---------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+---------------------------------+--------------+-----------+----------------+------------------+---------------------------------+------+------+------+------+------+------+----------+---------------+-------------------------+--------------+------------+----------------+-------------------------+
```

`garmin-edit show message offset <file>` would display an offset in the file for each field:

```
====  Message #0 (file_id)  ====
+-----------------+----------------+------------+----------------+------------+------------+------------+
| serial_number 3 | time_created 4 |          7 | manufacturer 1 |  product 2 |   number 5 |     type 0 |
+-----------------+----------------+------------+----------------+------------+------------+------------+
|             @42 |            @46 |        @50 |            @54 |        @56 |        @58 |        @60 |
|      XXXXXXXXXX |     1109429867 | 4294967295 |              1 |       XXXX |      65535 |          4 |
+-----------------+----------------+------------+----------------+------------+------------+------------+

```


Filtering particular message(s) can be achieved by passing message name(s) (or the global number(s)). Several message names should be separated by `|`:

`garmin-edit show message "file_id|record" <filename>`.

An example output:

```
BinaryScanner::scan: 13888 data messages
====  Message #0 (file_id)  ====
+-----------------+---------------------+------------+----------------+-----------+----------+--------+
| serial_number 3 |      time_created 4 |          7 | manufacturer 1 | product 2 | number 5 | type 0 |
+-----------------+---------------------+------------+----------------+-----------+----------+--------+
|      XXXXXXXXXX | 2025-02-25 16:57:47 | 4294967295 |              1 |      XXXX |    65535 |      4 |
+-----------------+---------------------+------------+----------------+-----------+----------+--------+

====  Message #20 (record)  ====
+---------------------+----------------+-----------------+------------+-------------------+----------------------+-------------------+--------------+-----------+----------------+-----------------------+------+------+------+------+------+
|       timestamp 253 | position_lat 0 | position_long 1 | distance 5 | enhanced_speed 73 | enhanced_altitude 78 | cycle_length16 87 | heart_rate 3 | cadence 4 | temperature 13 | fractional_cadence 53 |  107 |  134 |  135 |  136 |  143 |
+---------------------+----------------+-----------------+------------+-------------------+----------------------+-------------------+--------------+-----------+----------------+-----------------------+------+------+------+------+------+
| 2025-02-25 16:57:47 |      655026973 |       285844461 |          0 |                 0 |                 2832 |                 0 |           94 |         0 |             22 |                     0 |    0 |  255 |  114 |   94 |   26 |
| 2025-02-25 16:57:52 |      655026409 |       285844597 |        535 |              1838 |                 2832 |             65369 |           94 |         0 |             22 |                     0 |    1 |  255 |  129 |   94 |   26 |
| 2025-02-25 16:57:58 |      655025540 |       285845495 |       1471 |              1624 |                 2831 |             65369 |           94 |        59 |             22 |                    64 |    1 |  255 |  139 |   94 |   26 |
| 2025-02-25 16:58:06 |      655024505 |       285846663 |       2654 |              1502 |                 2832 |                 0 |           96 |        59 |             22 |                     0 |    1 |  255 |  118 |   96 |   26 |
...
| 2025-02-25 18:34:59 |      655027144 |       285844518 |     898249 |              1726 |                 2860 |                 0 |          110 |        57 |             11 |                    64 |    1 |  255 |  102 |  110 |   22 |
+---------------------+----------------+-----------------+------------+-------------------+----------------------+-------------------+--------------+-----------+----------------+-----------------------+------+------+------+------+------+
```

If "raw" option is given, timestamps would be displayed in original format. I.e. `garmin-edit show message raw <file name>`:

```
BinaryScanner::scan: 13888 data messages
====  Message #0 (file_id)  ====
+-----------------+----------------+------------+----------------+-----------+----------+--------+
| serial_number 3 | time_created 4 |          7 | manufacturer 1 | product 2 | number 5 | type 0 |
+-----------------+----------------+------------+----------------+-----------+----------+--------+
|      3509228688 |     1109429867 | 4294967295 |              1 |      4575 |    65535 |      4 |
+-----------------+----------------+------------+----------------+-----------+----------+--------+
```

Fields of interest can be filtered too. But only for _one_ message. It would be possible to match messages/fields, but at the moment this is not yet possible - maybe in the future. So for now one can filter _one_ message and filter fields for it:

`garmin-edit show message session "timestamp|start_time" <file name>`

And the result will be the Session message with only those two fields:

```
Garmin FIT file editor/analyzer version 1.0.5
BinaryScanner::scan: 8 data messages
====  Message #18 (session)  ====
+---------------------+---------------------+
|       timestamp 253 |        start_time 2 |
+---------------------+---------------------+
| 2025-03-11 15:24:51 | 2025-03-11 15:23:51 |
+---------------------+---------------------+
```

Additional option can be used for showing coordinates in degrees, because Garmin uses "semicircles", that are `int32_t` integer representation of the coordinates. One unit equals to approximately 11 mm at the equator - more than sufficient for a casual user.

`garmin-edit show message degrees record "timestamp|position_lat|position_long" <file name>`

Shows:

```
====  Message #20 (record)  ====
+---------------------+----------------+-----------------+
|       timestamp 253 | position_lat 0 | position_long 1 |
+---------------------+----------------+-----------------+
| 2025-02-15 13:58:39 |     13.1576901 |       79.118657 |
| 2025-02-15 13:58:40 |     13.1576848 |      79.1186541 |
| 2025-02-15 13:58:41 |     13.1576818 |      79.1186524 |
| 2025-02-15 13:58:42 |     13.1576773 |      79.1186493 |
| 2025-02-15 13:58:43 |     13.1576754 |      79.1186485 |
| 2025-02-15 13:58:44 |     13.1576743 |      79.1186463 |
| 2025-02-15 13:58:45 |     13.1576728 |      79.1186448 |
| 2025-02-15 13:58:46 |     13.1576735 |      79.1186452 |
| 2025-02-15 13:58:47 |     13.1576754 |      79.1186483 |
| 2025-02-15 13:58:48 |     13.1576753 |      79.1186479 |
| 2025-02-15 13:58:49 |     13.1576747 |       79.118647 |
| 2025-02-15 13:58:50 |     13.1576726 |      79.1186481 |
| 2025-02-15 13:58:51 |     13.1576736 |      79.1186515 |
| 2025-02-15 13:58:52 |     13.1576753 |      79.1186534 |
| 2025-02-15 13:58:53 |     13.1576662 |       79.118657 |
| 2025-02-15 13:58:54 |     13.1576557 |      79.1186585 |
...
| 2025-02-15 15:02:43 |     13.1575804 |       79.118584 |
+---------------------+----------------+-----------------+
```

### Listing Activities

A separate part, currently under development and still in rudimentary state, is a command `garmin-edit show activities <directory|file>`.

It either recursively scans a given directory or just parses one file and provides a (currently very basic) table with found activities:

```
Found 78 activities.
+-------------------------+---------------+-------------------+
|               File Name | Activity Name |             Sport |
+-------------------------+---------------+-------------------+
| 2025-03-01-07-36-02.fit |        Ėjimas |           WALKING |
| 2025-03-01-21-15-14.fit |     Stopwatch |         STOPWATCH |
| 2025-03-02-19-15-43.fit |          Jėga | STRENGTH_TRAINING |
| 2025-03-03-06-43-56.fit |       Bėgimas |           RUNNING |
| 2025-03-03-12-16-32.fit |          Joga |              YOGA |
| 2025-03-03-15-22-37.fit |        Ėjimas |           WALKING |
| 2025-03-03-19-02-06.fit |     Stopwatch |         STOPWATCH |
| 2025-03-04-06-34-20.fit |       Bėgimas |           RUNNING |
...
| 2025-03-31-15-41-24.fit |        Ėjimas |           WALKING |
| 2025-03-31-18-07-20.fit |     Stopwatch |         STOPWATCH |
+-------------------------+---------------+-------------------+
```

With time I'll include more details here, similar to the `Activities > All Activities` in the Garmin Connect.

### Replacing Coordinates from GPX

If one want's to collect various events badges (e.g. Olathe Marathon), there are two ways:
1. Run in place.
2. Run anywhere the required distance and replace coordinates with the event's course.

`garmin-edit replace gpx [session|advanced] <fit file> <gpx file> <new fit file>`

And that's it. Nothing else will be replaced, not even the elevation data (however, I might add this feature later).

`session` argument gives a possibility to update _only_ the session record, i.e. the "place" of the FIT file, but leave the original track. This _sometimes_ works on some events - apparently, depends on how Garmin checks the attendance.

By default, coordinate points from GPX track are overwritten in the FIT file by approximate fitting, i.e. if a record in the FIT file has a shorter distance, than the next GPX point, the current GPX point is used.

`advanced` argument makes it more realistic - it extrapolates intermediate points when GPX track has less points than the FIT file (but the same or greater distance). It calculates the bearing, the distance between the points and extrapolates them. If there are more points in the GPX file, extrapolation is used to get the corresponding distance. So this is more accurate and would reflect the time/distance relation when reviewing the activity in Connect or anywhere else.

### NOTES on Undocumented Messages

There are quite many undocumented messages, that are not described in the FIT Profile. Strangely, message #104 is also not present, but it shows battery percentage decline during the workout in the field #2. Field #0 is the battery voltage (I think):

```
$ garmin-edit show message 104 2025-03-16-20-45-15-3498.fit
Garmin FIT file editor/analyzer version 1.0.4
BinaryScanner::scan: 4285 data messages
====  Message #104  ====
+---------------------+-------------+-------+-----+------+
|                 253 |           4 |     0 |   2 |    3 |
+---------------------+-------------+-------+-----+------+
| 2025-03-16 20:45:39 |           0 |  4073 |  79 |   31 |
| 2025-03-16 20:50:39 |           0 |  4076 |  79 |   32 |
| 2025-03-16 20:55:39 |           0 |  4071 |  78 |   32 |
| 2025-03-16 21:00:39 |           0 |  4066 |  78 |   32 |
| 2025-03-16 21:05:39 |           0 |  4063 |  77 |   32 |
| 2025-03-16 21:10:39 |           0 |  4057 |  77 |   32 |
| 2025-03-16 21:15:39 |           0 |  4053 |  76 |   32 |
+---------------------+-------------+-------+-----+------+
```

Message #147 shows all paired sensors (even irrelevant to the activity, like Yoga in this case):

```
$ garmin-edit show message 147 2025-03-16-20-45-15-3498.fit
Garmin FIT file editor/analyzer version 1.0.4
BinaryScanner::scan: 4285 data messages
====  Message #147  ====
+------------+------------------+------------+-----+-----+------------+------------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|          0 |                2 |         13 |  18 |  20 |         64 |         65 |   254 |    10 |    11 |    21 |    25 |    26 |    32 |    33 |    34 |    35 |    40 |    55 |    56 |    57 |    73 |    85 |   1 |   3 |   4 |   5 |   6 |   7 |   9 |  12 |  14 |  15 |  16 |  17 |  19 |  24 |  31 |  36 |  37 |  38 |  39 |  41 |  42 |  43 |  44 |  45 |  46 |  47 |  48 |  50 |  51 |  52 |  53 |  58 |  59 |  60 |  61 |  62 |  63 |  82 |  86 |
+------------+------------------+------------+-----+-----+------------+------------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
|   24695821 |             OH1+ | 4294967295 |   0 |   0 | 4294967295 | 4294967295 |     0 | 65535 | 65535 | 65535 | 65535 | 65535 |     1 | 65535 |   100 | 65535 |     0 | 65535 | 65535 | 65535 |     1 | 65535 |   0 |   1 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 |   0 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 | 255 | 255 | 255 | 255 | 255 |   0 |   0 | 255 | 255 | 255 | 255 | 255 |   0 | 255 | 255 | 255 |
|   92033278 |            Stryd | 4294967295 |   0 |   0 | 4294967295 | 4294967295 |     1 | 65535 |  1042 | 65535 | 65535 | 65535 |  4660 | 65535 |    10 | 65535 |     0 | 65535 | 65535 | 65535 |     1 | 65535 |   0 |   1 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 |   0 |   0 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 |   2 |   1 | 255 | 255 | 255 |   0 |   2 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |
| 1903838343 |      CAD-0476295 | 4294967295 |   0 |   0 | 4294967295 | 4294967295 |     2 | 65535 | 65535 | 65535 | 65535 | 65535 |    20 | 65535 |   240 | 65535 |     0 | 65535 | 65535 | 65535 |     3 | 65535 |   0 |   1 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 |   0 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 | 255 | 255 | 255 | 255 |  70 |   0 |   1 |   1 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |
|  561720578 |      SPD-0142594 | 4294967295 |   0 |   0 | 4294967295 | 4294967295 |     3 |  2068 | 65535 |  2122 | 65535 | 65535 |  3192 | 65535 |   410 | 65535 |     0 | 65535 | 65535 | 65535 |     3 | 65535 |   0 |   1 | 255 | 255 |   0 | 255 | 255 | 255 | 255 | 255 | 255 |   0 |   0 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 | 255 | 255 | 255 | 255 | 109 |   0 |   4 |   1 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |
|  622428666 |           164346 | 4294967295 |   0 |   0 | 4294967295 | 4294967295 |     4 | 65535 | 65535 | 65535 | 65535 | 65535 | 65535 | 65535 | 65535 | 65535 |     0 | 65535 | 65535 | 65535 |     1 | 65535 |   0 |   1 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 |   0 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 | 255 | 255 | 255 | 255 | 255 |   0 |   6 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |
|          0 |   31.1E.05.10.E8 | 4294967295 |   0 |   0 | 4294967295 | 4294967295 |     5 | 65535 | 65535 | 65535 | 65535 | 65535 | 65535 | 65535 | 65535 | 65535 |     0 | 65535 | 65535 | 65535 |     2 | 65535 | 255 |   1 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 |   0 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 | 255 |   0 |   1 |   1 | 255 | 255 | 147 |   1 |   0 |   1 | 255 | 255 | 255 | 255 |   0 | 255 | 255 | 255 |
+------------+------------------+------------+-----+-----+------------+------------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
```

### Raw Editing

To replace names or some particular fields it is possible now to set uint16_t, uint32_t and string values by providing offset of a particular field:

`garmin-edit set raw s 1803 "Long walk" <file name>`

`sport` and `session` messages have names, that can be modified!

## Garmin Disconnect

Surely, world is GUIed for a long time now. Developing GUI applications was always a burden because of a complicated layouts, event management etc. Some GUI designing applications are there, but they also have their limitations. Thankfully, some Bigtechs developed powerful AI helpers that can be employed exactly for this purpose, i.e. generating UI code.

Garmin Disconnect may replace some functionality of the Garmin Connect (hence the name), but fully locally working with FIT files. It won't ever be a very sophisticated package (like Golden Cheetah, for example), but it provides simple functions for local FIT file management.

### Features

**Activity Management:**
- Browse FIT files by directory tree or time hierarchy (Year/Month/Day/Hour)
- Activities panel with sortable columns: Date, Name, Sport, Duration, Distance, Pace/Speed, Heart Rate
- **Metadata caching** - Fast loading via `.meta` sidecar files (100-1000x faster on subsequent scans)
- **Edit activity names** with F2 key - names are preserved in cache
- Drag & drop support for file operations - out of the application only
- Context menu integration with file managers

**Map Visualization:**
- OpenStreetMap-based track rendering with Mapnik
- Support for PBF and Spatialite map formats
- Automatic PBF to Spatialite conversion (requires `gdal-bin`)
- Zoom, pan, and track-fit controls
- Custom map styling support

**Product/Device Editing:**
- Change device product ID in FIT files
- Maintains file integrity with automatic CRC recalculation

**Configuration:**
- Settings stored in `~/.config/garmin-disconnect` (Linux) or registry (Windows)
- Customizable data directory path via `data_directory` setting
- Standard system locations: `/usr/share/garmin-disconnect/data` (Linux), `%PROGRAMDATA%\garmin-disconnect\data` (Windows)

More functions will be added little by little.

# Building

I'm not going to release the binaries, so compile at your own leisure.

## Dependencies

**Required:**
1. **[FIT SDK](https://developer.garmin.com/fit/download/)** - download it and place at the same directory level as this project
2. **Pugixml** - `libpugixml-dev` on Linux
3. **wxWidgets** - `libwxgtk3.2-dev` on Linux (version 3.2+)
4. **Mapnik** - `libmapnik-dev` on Linux (for map rendering in GUI)

**Recommended:**
5. **GDAL** - `gdal-bin` on Linux (provides `ogr2ogr` for PBF to Spatialite conversion)

**Suggested:**
6. **Osmium** - `osmium-tool` on Linux (for OSM data processing)

The directory structure should be as follows (but adjust for the SDK version):

```
.
├── FitSDKRelease_21.171.00
└── GarminFitUtilities
```

If you have different SDK version, edit the top `CMakeLists.txt` file in the `GarminFitUtilities`.

If you don't require all three (currently) utilities, there are options in the top `CMakeLists.txt` file to turn their compilation off.

Then:
* `cd GarminFitUtilities`
* `mkdir build`
* `cmake -B build`
* `cmake --build build`

*NOTE:* All C++ FIT SDK is compiled as a shared library. Resulting utilities will have it's path compiled in them (`set(CMAKE_SKIP_RPATH TRUE)` is commented out).

# Debian Package

Above step may be skipped (except for the required dependencies) and `build-deb.sh` can be used to create a full Debian package for installation.


Have fun!
