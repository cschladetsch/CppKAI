// Copyright (c) 2015, Uduse - Aedi Wang, All rights reserved.
// This file is licensed under The BSD 2-Clause License, see LICENSE.md for
// details
/*!
 * # Escape-Sequence-Color-Header
 * Color Your Output in Unix!!
 *
 *
 * ##File:
 * Escape Sequence Color Header.h
 *
 * ##Date:
 * 2015/02/09 16:13 (~3h)
 *
 * ##Author:
 * Uduse
 *
 *
 *
 * ##Text Attribute Options:
 * ATTRIBUTES_OFF, BOLD, UNDERSCORE, BLINK, REVERSE_VIDEO, CONCEALED
 *
 * ##Color Options:
 * BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
 *
 *
 * ##Format:
 *
 * General Format, include value you want in $variable$
 *
 * COLOR_$Foreground Color$_$Background Color$
 * COLOR_$Text Attribute$_$Foreground Color$_$Background Color$
 * COLOR_NORMAL  // To set color to default
 *
 *
 * e.g.
 *
 * COLOR_BLUE_BLACK // Leave Text Attribute Blank if no Text Attribute appied
 * COLOR_UNDERSCORE_YELLOW_RED
 * COLOR_NORMAL
 *
 *
 * ##Usage:
 *
 * Just use  to stream the color you want before outputting text and
 * use  again to set the color to normal after outputting text.
 *
 * cout << COLOR_BLUE_BLACK << "TEXT" << COLOR_NORMAL << endl;
 * cout << COLOR_BOLD_YELLOW_CYAN << "TEXT" << COLOR_NORMAL << endl;
 * */
const char COLOR_NORMAL[] = {0x1b, '[', '0', ';', '3', '9', 'm', 0};
const char COLOR_NORMAL_COLOREND[] = {0x1b, '[', '0', ';', '3', '9', 'm', 0};
const char COLOR_ATTRIBUTES_OFF[] = {0x1b, '[', '0', 'm', 0};
const char COLOR_BOLD[] = {0x1b, '[', '1', 'm', 0};
const char COLOR_UNDERSCORE[] = {0x1b, '[', '4', 'm', 0};
const char COLOR_BLINK[] = {0x1b, '[', '5', 'm', 0};
const char COLOR_REVERSEVIDEO[] = {0x1b, '[', '7', 'm', 0};
const char COLOR_CONCEALED[] = {0x1b, '[', '8', 'm', 0};
const char COLOR_BLACK_BLACK[] = {0x1b, '[', '3', '0', ';', '4', '0', 'm', 0};
const char COLOR_BLACK_RED[] = {0x1b, '[', '3', '0', ';', '4', '1', 'm', 0};
const char COLOR_BLACK_GREEN[] = {0x1b, '[', '3', '0', ';', '4', '2', 'm', 0};
const char COLOR_BLACK_YELLOW[] = {0x1b, '[', '3', '0', ';', '4', '3', 'm', 0};
const char COLOR_BLACK_BLUE[] = {0x1b, '[', '3', '0', ';', '4', '4', 'm', 0};
const char COLOR_BLACK_MAGENTA[] = {0x1b, '[', '3', '0', ';', '4', '5', 'm', 0};
const char COLOR_BLACK_CYAN[] = {0x1b, '[', '3', '0', ';', '4', '6', 'm', 0};
const char COLOR_BLACK_WHITE[] = {0x1b, '[', '3', '0', ';', '4', '7', 'm', 0};
const char COLOR_BLACK_NORMAL[] = {0x1b, '[', '3', '0', ';', '4', '9', 'm', 0};
const char COLOR_RED_BLACK[] = {0x1b, '[', '3', '1', ';', '4', '0', 'm', 0};
const char COLOR_RED_RED[] = {0x1b, '[', '3', '1', ';', '4', '1', 'm', 0};
const char COLOR_RED_GREEN[] = {0x1b, '[', '3', '1', ';', '4', '2', 'm', 0};
const char COLOR_RED_YELLOW[] = {0x1b, '[', '3', '1', ';', '4', '3', 'm', 0};
const char COLOR_RED_BLUE[] = {0x1b, '[', '3', '1', ';', '4', '4', 'm', 0};
const char COLOR_RED_MAGENTA[] = {0x1b, '[', '3', '1', ';', '4', '5', 'm', 0};
const char COLOR_RED_CYAN[] = {0x1b, '[', '3', '1', ';', '4', '6', 'm', 0};
const char COLOR_RED_WHITE[] = {0x1b, '[', '3', '1', ';', '4', '7', 'm', 0};
const char COLOR_RED_NORMAL[] = {0x1b, '[', '3', '1', ';', '4', '9', 'm', 0};
const char COLOR_GREEN_BLACK[] = {0x1b, '[', '3', '2', ';', '4', '0', 'm', 0};
const char COLOR_GREEN_RED[] = {0x1b, '[', '3', '2', ';', '4', '1', 'm', 0};
const char COLOR_GREEN_GREEN[] = {0x1b, '[', '3', '2', ';', '4', '2', 'm', 0};
const char COLOR_GREEN_YELLOW[] = {0x1b, '[', '3', '2', ';', '4', '3', 'm', 0};
const char COLOR_GREEN_BLUE[] = {0x1b, '[', '3', '2', ';', '4', '4', 'm', 0};
const char COLOR_GREEN_MAGENTA[] = {0x1b, '[', '3', '2', ';', '4', '5', 'm', 0};
const char COLOR_GREEN_CYAN[] = {0x1b, '[', '3', '2', ';', '4', '6', 'm', 0};
const char COLOR_GREEN_WHITE[] = {0x1b, '[', '3', '2', ';', '4', '7', 'm', 0};
const char COLOR_GREEN_NORMAL[] = {0x1b, '[', '3', '2', ';', '4', '9', 'm', 0};
const char COLOR_YELLOW_BLACK[] = {0x1b, '[', '3', '3', ';', '4', '0', 'm', 0};
const char COLOR_YELLOW_RED[] = {0x1b, '[', '3', '3', ';', '4', '1', 'm', 0};
const char COLOR_YELLOW_GREEN[] = {0x1b, '[', '3', '3', ';', '4', '2', 'm', 0};
const char COLOR_YELLOW_YELLOW[] = {0x1b, '[', '3', '3', ';', '4', '3', 'm', 0};
const char COLOR_YELLOW_BLUE[] = {0x1b, '[', '3', '3', ';', '4', '4', 'm', 0};
const char COLOR_YELLOW_MAGENTA[] = {0x1b, '[', '3', '3', ';',
                                     '4',  '5', 'm', 0};
const char COLOR_YELLOW_CYAN[] = {0x1b, '[', '3', '3', ';', '4', '6', 'm', 0};
const char COLOR_YELLOW_WHITE[] = {0x1b, '[', '3', '3', ';', '4', '7', 'm', 0};
const char COLOR_YELLOW_NORMAL[] = {0x1b, '[', '3', '3', ';', '4', '9', 'm', 0};
const char COLOR_BLUE_BLACK[] = {0x1b, '[', '3', '4', ';', '4', '0', 'm', 0};
const char COLOR_BLUE_RED[] = {0x1b, '[', '3', '4', ';', '4', '1', 'm', 0};
const char COLOR_BLUE_GREEN[] = {0x1b, '[', '3', '4', ';', '4', '2', 'm', 0};
const char COLOR_BLUE_YELLOW[] = {0x1b, '[', '3', '4', ';', '4', '3', 'm', 0};
const char COLOR_BLUE_BLUE[] = {0x1b, '[', '3', '4', ';', '4', '4', 'm', 0};
const char COLOR_BLUE_MAGENTA[] = {0x1b, '[', '3', '4', ';', '4', '5', 'm', 0};
const char COLOR_BLUE_CYAN[] = {0x1b, '[', '3', '4', ';', '4', '6', 'm', 0};
const char COLOR_BLUE_WHITE[] = {0x1b, '[', '3', '4', ';', '4', '7', 'm', 0};
const char COLOR_BLUE_NORMAL[] = {0x1b, '[', '3', '4', ';', '4', '9', 'm', 0};
const char COLOR_MAGENTA_BLACK[] = {0x1b, '[', '3', '5', ';', '4', '0', 'm', 0};
const char COLOR_MAGENTA_RED[] = {0x1b, '[', '3', '5', ';', '4', '1', 'm', 0};
const char COLOR_MAGENTA_GREEN[] = {0x1b, '[', '3', '5', ';', '4', '2', 'm', 0};
const char COLOR_MAGENTA_YELLOW[] = {0x1b, '[', '3', '5', ';',
                                     '4',  '3', 'm', 0};
const char COLOR_MAGENTA_BLUE[] = {0x1b, '[', '3', '5', ';', '4', '4', 'm', 0};
const char COLOR_MAGENTA_MAGENTA[] = {0x1b, '[', '3', '5', ';',
                                      '4',  '5', 'm', 0};
const char COLOR_MAGENTA_CYAN[] = {0x1b, '[', '3', '5', ';', '4', '6', 'm', 0};
const char COLOR_MAGENTA_WHITE[] = {0x1b, '[', '3', '5', ';', '4', '7', 'm', 0};
const char COLOR_MAGENTA_NORMAL[] = {0x1b, '[', '3', '5', ';',
                                     '4',  '9', 'm', 0};
const char COLOR_CYAN_BLACK[] = {0x1b, '[', '3', '6', ';', '4', '0', 'm', 0};
const char COLOR_CYAN_RED[] = {0x1b, '[', '3', '6', ';', '4', '1', 'm', 0};
const char COLOR_CYAN_GREEN[] = {0x1b, '[', '3', '6', ';', '4', '2', 'm', 0};
const char COLOR_CYAN_YELLOW[] = {0x1b, '[', '3', '6', ';', '4', '3', 'm', 0};
const char COLOR_CYAN_BLUE[] = {0x1b, '[', '3', '6', ';', '4', '4', 'm', 0};
const char COLOR_CYAN_MAGENTA[] = {0x1b, '[', '3', '6', ';', '4', '5', 'm', 0};
const char COLOR_CYAN_CYAN[] = {0x1b, '[', '3', '6', ';', '4', '6', 'm', 0};
const char COLOR_CYAN_WHITE[] = {0x1b, '[', '3', '6', ';', '4', '7', 'm', 0};
const char COLOR_CYAN_NORMAL[] = {0x1b, '[', '3', '6', ';', '4', '9', 'm', 0};
const char COLOR_WHITE_BLACK[] = {0x1b, '[', '3', '7', ';', '4', '0', 'm', 0};
const char COLOR_WHITE_RED[] = {0x1b, '[', '3', '7', ';', '4', '1', 'm', 0};
const char COLOR_WHITE_GREEN[] = {0x1b, '[', '3', '7', ';', '4', '2', 'm', 0};
const char COLOR_WHITE_YELLOW[] = {0x1b, '[', '3', '7', ';', '4', '3', 'm', 0};
const char COLOR_WHITE_BLUE[] = {0x1b, '[', '3', '7', ';', '4', '4', 'm', 0};
const char COLOR_WHITE_MAGENTA[] = {0x1b, '[', '3', '7', ';', '4', '5', 'm', 0};
const char COLOR_WHITE_CYAN[] = {0x1b, '[', '3', '7', ';', '4', '6', 'm', 0};
const char COLOR_WHITE_WHITE[] = {0x1b, '[', '3', '7', ';', '4', '7', 'm', 0};
const char COLOR_WHITE_NORMAL[] = {0x1b, '[', '3', '7', ';', '4', '9', 'm', 0};
const char COLOR_NORMAL_BLACK[] = {0x1b, '[', '3', '9', ';', '4', '0', 'm', 0};
const char COLOR_NORMAL_RED[] = {0x1b, '[', '3', '9', ';', '4', '1', 'm', 0};
const char COLOR_NORMAL_GREEN[] = {0x1b, '[', '3', '9', ';', '4', '2', 'm', 0};
const char COLOR_NORMAL_YELLOW[] = {0x1b, '[', '3', '9', ';', '4', '3', 'm', 0};
const char COLOR_NORMAL_BLUE[] = {0x1b, '[', '3', '9', ';', '4', '4', 'm', 0};
const char COLOR_NORMAL_MAGENTA[] = {0x1b, '[', '3', '9', ';',
                                     '4',  '5', 'm', 0};
const char COLOR_NORMAL_CYAN[] = {0x1b, '[', '3', '9', ';', '4', '6', 'm', 0};
const char COLOR_NORMAL_WHITE[] = {0x1b, '[', '3', '9', ';', '4', '7', 'm', 0};
const char COLOR_NORMAL_NORMAL[] = {0x1b, '[', '3', '9', ';', '4', '9', 'm', 0};
const char COLOR_BOLD_BLACK_BLACK[] = {0x1b, '[', '1', ';', '3', '0',
                                       ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_BLACK_RED[] = {0x1b, '[', '1', ';', '3', '0',
                                     ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_BLACK_GREEN[] = {0x1b, '[', '1', ';', '3', '0',
                                       ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_BLACK_YELLOW[] = {0x1b, '[', '1', ';', '3', '0',
                                        ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_BLACK_BLUE[] = {0x1b, '[', '1', ';', '3', '0',
                                      ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_BLACK_MAGENTA[] = {0x1b, '[', '1', ';', '3', '0',
                                         ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_BLACK_CYAN[] = {0x1b, '[', '1', ';', '3', '0',
                                      ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_BLACK_WHITE[] = {0x1b, '[', '1', ';', '3', '0',
                                       ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_BLACK_NORMAL[] = {0x1b, '[', '1', ';', '3', '0',
                                        ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_RED_BLACK[] = {0x1b, '[', '1', ';', '3', '1',
                                     ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_RED_RED[] = {0x1b, '[', '1', ';', '3', '1',
                                   ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_RED_GREEN[] = {0x1b, '[', '1', ';', '3', '1',
                                     ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_RED_YELLOW[] = {0x1b, '[', '1', ';', '3', '1',
                                      ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_RED_BLUE[] = {0x1b, '[', '1', ';', '3', '1',
                                    ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_RED_MAGENTA[] = {0x1b, '[', '1', ';', '3', '1',
                                       ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_RED_CYAN[] = {0x1b, '[', '1', ';', '3', '1',
                                    ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_RED_WHITE[] = {0x1b, '[', '1', ';', '3', '1',
                                     ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_RED_NORMAL[] = {0x1b, '[', '1', ';', '3', '1',
                                      ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_GREEN_BLACK[] = {0x1b, '[', '1', ';', '3', '2',
                                       ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_GREEN_RED[] = {0x1b, '[', '1', ';', '3', '2',
                                     ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_GREEN_GREEN[] = {0x1b, '[', '1', ';', '3', '2',
                                       ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_GREEN_YELLOW[] = {0x1b, '[', '1', ';', '3', '2',
                                        ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_GREEN_BLUE[] = {0x1b, '[', '1', ';', '3', '2',
                                      ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_GREEN_MAGENTA[] = {0x1b, '[', '1', ';', '3', '2',
                                         ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_GREEN_CYAN[] = {0x1b, '[', '1', ';', '3', '2',
                                      ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_GREEN_WHITE[] = {0x1b, '[', '1', ';', '3', '2',
                                       ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_GREEN_NORMAL[] = {0x1b, '[', '1', ';', '3', '2',
                                        ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_YELLOW_BLACK[] = {0x1b, '[', '1', ';', '3', '3',
                                        ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_YELLOW_RED[] = {0x1b, '[', '1', ';', '3', '3',
                                      ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_YELLOW_GREEN[] = {0x1b, '[', '1', ';', '3', '3',
                                        ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_YELLOW_YELLOW[] = {0x1b, '[', '1', ';', '3', '3',
                                         ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_YELLOW_BLUE[] = {0x1b, '[', '1', ';', '3', '3',
                                       ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_YELLOW_MAGENTA[] = {0x1b, '[', '1', ';', '3', '3',
                                          ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_YELLOW_CYAN[] = {0x1b, '[', '1', ';', '3', '3',
                                       ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_YELLOW_WHITE[] = {0x1b, '[', '1', ';', '3', '3',
                                        ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_YELLOW_NORMAL[] = {0x1b, '[', '1', ';', '3', '3',
                                         ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_BLUE_BLACK[] = {0x1b, '[', '1', ';', '3', '4',
                                      ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_BLUE_RED[] = {0x1b, '[', '1', ';', '3', '4',
                                    ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_BLUE_GREEN[] = {0x1b, '[', '1', ';', '3', '4',
                                      ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_BLUE_YELLOW[] = {0x1b, '[', '1', ';', '3', '4',
                                       ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_BLUE_BLUE[] = {0x1b, '[', '1', ';', '3', '4',
                                     ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_BLUE_MAGENTA[] = {0x1b, '[', '1', ';', '3', '4',
                                        ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_BLUE_CYAN[] = {0x1b, '[', '1', ';', '3', '4',
                                     ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_BLUE_WHITE[] = {0x1b, '[', '1', ';', '3', '4',
                                      ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_BLUE_NORMAL[] = {0x1b, '[', '1', ';', '3', '4',
                                       ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_MAGENTA_BLACK[] = {0x1b, '[', '1', ';', '3', '5',
                                         ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_MAGENTA_RED[] = {0x1b, '[', '1', ';', '3', '5',
                                       ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_MAGENTA_GREEN[] = {0x1b, '[', '1', ';', '3', '5',
                                         ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_MAGENTA_YELLOW[] = {0x1b, '[', '1', ';', '3', '5',
                                          ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_MAGENTA_BLUE[] = {0x1b, '[', '1', ';', '3', '5',
                                        ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_MAGENTA_MAGENTA[] = {0x1b, '[', '1', ';', '3', '5',
                                           ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_MAGENTA_CYAN[] = {0x1b, '[', '1', ';', '3', '5',
                                        ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_MAGENTA_WHITE[] = {0x1b, '[', '1', ';', '3', '5',
                                         ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_MAGENTA_NORMAL[] = {0x1b, '[', '1', ';', '3', '5',
                                          ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_CYAN_BLACK[] = {0x1b, '[', '1', ';', '3', '6',
                                      ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_CYAN_RED[] = {0x1b, '[', '1', ';', '3', '6',
                                    ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_CYAN_GREEN[] = {0x1b, '[', '1', ';', '3', '6',
                                      ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_CYAN_YELLOW[] = {0x1b, '[', '1', ';', '3', '6',
                                       ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_CYAN_BLUE[] = {0x1b, '[', '1', ';', '3', '6',
                                     ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_CYAN_MAGENTA[] = {0x1b, '[', '1', ';', '3', '6',
                                        ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_CYAN_CYAN[] = {0x1b, '[', '1', ';', '3', '6',
                                     ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_CYAN_WHITE[] = {0x1b, '[', '1', ';', '3', '6',
                                      ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_CYAN_NORMAL[] = {0x1b, '[', '1', ';', '3', '6',
                                       ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_WHITE_BLACK[] = {0x1b, '[', '1', ';', '3', '7',
                                       ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_WHITE_RED[] = {0x1b, '[', '1', ';', '3', '7',
                                     ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_WHITE_GREEN[] = {0x1b, '[', '1', ';', '3', '7',
                                       ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_WHITE_YELLOW[] = {0x1b, '[', '1', ';', '3', '7',
                                        ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_WHITE_BLUE[] = {0x1b, '[', '1', ';', '3', '7',
                                      ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_WHITE_MAGENTA[] = {0x1b, '[', '1', ';', '3', '7',
                                         ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_WHITE_CYAN[] = {0x1b, '[', '1', ';', '3', '7',
                                      ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_WHITE_WHITE[] = {0x1b, '[', '1', ';', '3', '7',
                                       ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_WHITE_NORMAL[] = {0x1b, '[', '1', ';', '3', '7',
                                        ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_NORMAL_BLACK[] = {0x1b, '[', '1', ';', '3', '9',
                                        ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_NORMAL_RED[] = {0x1b, '[', '1', ';', '3', '9',
                                      ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_NORMAL_GREEN[] = {0x1b, '[', '1', ';', '3', '9',
                                        ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_NORMAL_YELLOW[] = {0x1b, '[', '1', ';', '3', '9',
                                         ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_NORMAL_BLUE[] = {0x1b, '[', '1', ';', '3', '9',
                                       ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_NORMAL_MAGENTA[] = {0x1b, '[', '1', ';', '3', '9',
                                          ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_NORMAL_CYAN[] = {0x1b, '[', '1', ';', '3', '9',
                                       ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_NORMAL_WHITE[] = {0x1b, '[', '1', ';', '3', '9',
                                        ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_NORMAL_NORMAL[] = {0x1b, '[', '1', ';', '3', '9',
                                         ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_BLACK[] = {0x1b, '[', '4', ';', '3', '0',
                                             ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_RED[] = {0x1b, '[', '4', ';', '3', '0',
                                           ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_GREEN[] = {0x1b, '[', '4', ';', '3', '0',
                                             ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_YELLOW[] = {0x1b, '[', '4', ';', '3', '0',
                                              ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_BLUE[] = {0x1b, '[', '4', ';', '3', '0',
                                            ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_MAGENTA[] = {0x1b, '[', '4', ';', '3', '0',
                                               ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_CYAN[] = {0x1b, '[', '4', ';', '3', '0',
                                            ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_WHITE[] = {0x1b, '[', '4', ';', '3', '0',
                                             ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_NORMAL[] = {0x1b, '[', '4', ';', '3', '0',
                                              ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_RED_BLACK[] = {0x1b, '[', '4', ';', '3', '1',
                                           ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_RED_RED[] = {0x1b, '[', '4', ';', '3', '1',
                                         ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_RED_GREEN[] = {0x1b, '[', '4', ';', '3', '1',
                                           ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_RED_YELLOW[] = {0x1b, '[', '4', ';', '3', '1',
                                            ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_RED_BLUE[] = {0x1b, '[', '4', ';', '3', '1',
                                          ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_RED_MAGENTA[] = {0x1b, '[', '4', ';', '3', '1',
                                             ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_RED_CYAN[] = {0x1b, '[', '4', ';', '3', '1',
                                          ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_RED_WHITE[] = {0x1b, '[', '4', ';', '3', '1',
                                           ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_RED_NORMAL[] = {0x1b, '[', '4', ';', '3', '1',
                                            ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_BLACK[] = {0x1b, '[', '4', ';', '3', '2',
                                             ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_RED[] = {0x1b, '[', '4', ';', '3', '2',
                                           ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_GREEN[] = {0x1b, '[', '4', ';', '3', '2',
                                             ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_YELLOW[] = {0x1b, '[', '4', ';', '3', '2',
                                              ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_BLUE[] = {0x1b, '[', '4', ';', '3', '2',
                                            ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_MAGENTA[] = {0x1b, '[', '4', ';', '3', '2',
                                               ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_CYAN[] = {0x1b, '[', '4', ';', '3', '2',
                                            ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_WHITE[] = {0x1b, '[', '4', ';', '3', '2',
                                             ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_NORMAL[] = {0x1b, '[', '4', ';', '3', '2',
                                              ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_BLACK[] = {0x1b, '[', '4', ';', '3', '3',
                                              ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_RED[] = {0x1b, '[', '4', ';', '3', '3',
                                            ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_GREEN[] = {0x1b, '[', '4', ';', '3', '3',
                                              ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_YELLOW[] = {0x1b, '[', '4', ';', '3', '3',
                                               ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_BLUE[] = {0x1b, '[', '4', ';', '3', '3',
                                             ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_MAGENTA[] = {0x1b, '[', '4', ';', '3', '3',
                                                ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_CYAN[] = {0x1b, '[', '4', ';', '3', '3',
                                             ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_WHITE[] = {0x1b, '[', '4', ';', '3', '3',
                                              ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_NORMAL[] = {0x1b, '[', '4', ';', '3', '3',
                                               ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_BLACK[] = {0x1b, '[', '4', ';', '3', '4',
                                            ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_RED[] = {0x1b, '[', '4', ';', '3', '4',
                                          ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_GREEN[] = {0x1b, '[', '4', ';', '3', '4',
                                            ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_YELLOW[] = {0x1b, '[', '4', ';', '3', '4',
                                             ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_BLUE[] = {0x1b, '[', '4', ';', '3', '4',
                                           ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_MAGENTA[] = {0x1b, '[', '4', ';', '3', '4',
                                              ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_CYAN[] = {0x1b, '[', '4', ';', '3', '4',
                                           ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_WHITE[] = {0x1b, '[', '4', ';', '3', '4',
                                            ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_NORMAL[] = {0x1b, '[', '4', ';', '3', '4',
                                             ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_BLACK[] = {0x1b, '[', '4', ';', '3', '5',
                                               ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_RED[] = {0x1b, '[', '4', ';', '3', '5',
                                             ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_GREEN[] = {0x1b, '[', '4', ';', '3', '5',
                                               ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_YELLOW[] = {0x1b, '[', '4', ';', '3', '5',
                                                ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_BLUE[] = {0x1b, '[', '4', ';', '3', '5',
                                              ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_MAGENTA[] = {0x1b, '[', '4', ';', '3', '5',
                                                 ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_CYAN[] = {0x1b, '[', '4', ';', '3', '5',
                                              ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_WHITE[] = {0x1b, '[', '4', ';', '3', '5',
                                               ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_NORMAL[] = {0x1b, '[', '4', ';', '3', '5',
                                                ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_BLACK[] = {0x1b, '[', '4', ';', '3', '6',
                                            ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_RED[] = {0x1b, '[', '4', ';', '3', '6',
                                          ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_GREEN[] = {0x1b, '[', '4', ';', '3', '6',
                                            ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_YELLOW[] = {0x1b, '[', '4', ';', '3', '6',
                                             ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_BLUE[] = {0x1b, '[', '4', ';', '3', '6',
                                           ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_MAGENTA[] = {0x1b, '[', '4', ';', '3', '6',
                                              ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_CYAN[] = {0x1b, '[', '4', ';', '3', '6',
                                           ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_WHITE[] = {0x1b, '[', '4', ';', '3', '6',
                                            ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_NORMAL[] = {0x1b, '[', '4', ';', '3', '6',
                                             ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_BLACK[] = {0x1b, '[', '4', ';', '3', '7',
                                             ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_RED[] = {0x1b, '[', '4', ';', '3', '7',
                                           ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_GREEN[] = {0x1b, '[', '4', ';', '3', '7',
                                             ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_YELLOW[] = {0x1b, '[', '4', ';', '3', '7',
                                              ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_BLUE[] = {0x1b, '[', '4', ';', '3', '7',
                                            ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_MAGENTA[] = {0x1b, '[', '4', ';', '3', '7',
                                               ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_CYAN[] = {0x1b, '[', '4', ';', '3', '7',
                                            ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_WHITE[] = {0x1b, '[', '4', ';', '3', '7',
                                             ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_NORMAL[] = {0x1b, '[', '4', ';', '3', '7',
                                              ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_BLACK[] = {0x1b, '[', '4', ';', '3', '9',
                                              ';',  '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_RED[] = {0x1b, '[', '4', ';', '3', '9',
                                            ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_GREEN[] = {0x1b, '[', '4', ';', '3', '9',
                                              ';',  '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_YELLOW[] = {0x1b, '[', '4', ';', '3', '9',
                                               ';',  '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_BLUE[] = {0x1b, '[', '4', ';', '3', '9',
                                             ';',  '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_MAGENTA[] = {0x1b, '[', '4', ';', '3', '9',
                                                ';',  '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_CYAN[] = {0x1b, '[', '4', ';', '3', '9',
                                             ';',  '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_WHITE[] = {0x1b, '[', '4', ';', '3', '9',
                                              ';',  '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_NORMAL[] = {0x1b, '[', '4', ';', '3', '9',
                                               ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_BLACK_BLACK[] = {0x1b, '[', '5', ';', '3', '0',
                                        ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_BLACK_RED[] = {0x1b, '[', '5', ';', '3', '0',
                                      ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_BLACK_GREEN[] = {0x1b, '[', '5', ';', '3', '0',
                                        ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_BLACK_YELLOW[] = {0x1b, '[', '5', ';', '3', '0',
                                         ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_BLACK_BLUE[] = {0x1b, '[', '5', ';', '3', '0',
                                       ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_BLACK_MAGENTA[] = {0x1b, '[', '5', ';', '3', '0',
                                          ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_BLACK_CYAN[] = {0x1b, '[', '5', ';', '3', '0',
                                       ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_BLACK_WHITE[] = {0x1b, '[', '5', ';', '3', '0',
                                        ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_BLACK_NORMAL[] = {0x1b, '[', '5', ';', '3', '0',
                                         ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_RED_BLACK[] = {0x1b, '[', '5', ';', '3', '1',
                                      ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_RED_RED[] = {0x1b, '[', '5', ';', '3', '1',
                                    ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_RED_GREEN[] = {0x1b, '[', '5', ';', '3', '1',
                                      ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_RED_YELLOW[] = {0x1b, '[', '5', ';', '3', '1',
                                       ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_RED_BLUE[] = {0x1b, '[', '5', ';', '3', '1',
                                     ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_RED_MAGENTA[] = {0x1b, '[', '5', ';', '3', '1',
                                        ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_RED_CYAN[] = {0x1b, '[', '5', ';', '3', '1',
                                     ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_RED_WHITE[] = {0x1b, '[', '5', ';', '3', '1',
                                      ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_RED_NORMAL[] = {0x1b, '[', '5', ';', '3', '1',
                                       ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_GREEN_BLACK[] = {0x1b, '[', '5', ';', '3', '2',
                                        ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_GREEN_RED[] = {0x1b, '[', '5', ';', '3', '2',
                                      ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_GREEN_GREEN[] = {0x1b, '[', '5', ';', '3', '2',
                                        ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_GREEN_YELLOW[] = {0x1b, '[', '5', ';', '3', '2',
                                         ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_GREEN_BLUE[] = {0x1b, '[', '5', ';', '3', '2',
                                       ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_GREEN_MAGENTA[] = {0x1b, '[', '5', ';', '3', '2',
                                          ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_GREEN_CYAN[] = {0x1b, '[', '5', ';', '3', '2',
                                       ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_GREEN_WHITE[] = {0x1b, '[', '5', ';', '3', '2',
                                        ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_GREEN_NORMAL[] = {0x1b, '[', '5', ';', '3', '2',
                                         ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_YELLOW_BLACK[] = {0x1b, '[', '5', ';', '3', '3',
                                         ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_YELLOW_RED[] = {0x1b, '[', '5', ';', '3', '3',
                                       ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_YELLOW_GREEN[] = {0x1b, '[', '5', ';', '3', '3',
                                         ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_YELLOW_YELLOW[] = {0x1b, '[', '5', ';', '3', '3',
                                          ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_YELLOW_BLUE[] = {0x1b, '[', '5', ';', '3', '3',
                                        ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_YELLOW_MAGENTA[] = {0x1b, '[', '5', ';', '3', '3',
                                           ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_YELLOW_CYAN[] = {0x1b, '[', '5', ';', '3', '3',
                                        ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_YELLOW_WHITE[] = {0x1b, '[', '5', ';', '3', '3',
                                         ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_YELLOW_NORMAL[] = {0x1b, '[', '5', ';', '3', '3',
                                          ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_BLUE_BLACK[] = {0x1b, '[', '5', ';', '3', '4',
                                       ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_BLUE_RED[] = {0x1b, '[', '5', ';', '3', '4',
                                     ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_BLUE_GREEN[] = {0x1b, '[', '5', ';', '3', '4',
                                       ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_BLUE_YELLOW[] = {0x1b, '[', '5', ';', '3', '4',
                                        ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_BLUE_BLUE[] = {0x1b, '[', '5', ';', '3', '4',
                                      ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_BLUE_MAGENTA[] = {0x1b, '[', '5', ';', '3', '4',
                                         ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_BLUE_CYAN[] = {0x1b, '[', '5', ';', '3', '4',
                                      ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_BLUE_WHITE[] = {0x1b, '[', '5', ';', '3', '4',
                                       ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_BLUE_NORMAL[] = {0x1b, '[', '5', ';', '3', '4',
                                        ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_MAGENTA_BLACK[] = {0x1b, '[', '5', ';', '3', '5',
                                          ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_MAGENTA_RED[] = {0x1b, '[', '5', ';', '3', '5',
                                        ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_MAGENTA_GREEN[] = {0x1b, '[', '5', ';', '3', '5',
                                          ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_MAGENTA_YELLOW[] = {0x1b, '[', '5', ';', '3', '5',
                                           ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_MAGENTA_BLUE[] = {0x1b, '[', '5', ';', '3', '5',
                                         ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_MAGENTA_MAGENTA[] = {0x1b, '[', '5', ';', '3', '5',
                                            ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_MAGENTA_CYAN[] = {0x1b, '[', '5', ';', '3', '5',
                                         ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_MAGENTA_WHITE[] = {0x1b, '[', '5', ';', '3', '5',
                                          ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_MAGENTA_NORMAL[] = {0x1b, '[', '5', ';', '3', '5',
                                           ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_CYAN_BLACK[] = {0x1b, '[', '5', ';', '3', '6',
                                       ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_CYAN_RED[] = {0x1b, '[', '5', ';', '3', '6',
                                     ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_CYAN_GREEN[] = {0x1b, '[', '5', ';', '3', '6',
                                       ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_CYAN_YELLOW[] = {0x1b, '[', '5', ';', '3', '6',
                                        ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_CYAN_BLUE[] = {0x1b, '[', '5', ';', '3', '6',
                                      ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_CYAN_MAGENTA[] = {0x1b, '[', '5', ';', '3', '6',
                                         ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_CYAN_CYAN[] = {0x1b, '[', '5', ';', '3', '6',
                                      ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_CYAN_WHITE[] = {0x1b, '[', '5', ';', '3', '6',
                                       ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_CYAN_NORMAL[] = {0x1b, '[', '5', ';', '3', '6',
                                        ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_WHITE_BLACK[] = {0x1b, '[', '5', ';', '3', '7',
                                        ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_WHITE_RED[] = {0x1b, '[', '5', ';', '3', '7',
                                      ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_WHITE_GREEN[] = {0x1b, '[', '5', ';', '3', '7',
                                        ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_WHITE_YELLOW[] = {0x1b, '[', '5', ';', '3', '7',
                                         ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_WHITE_BLUE[] = {0x1b, '[', '5', ';', '3', '7',
                                       ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_WHITE_MAGENTA[] = {0x1b, '[', '5', ';', '3', '7',
                                          ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_WHITE_CYAN[] = {0x1b, '[', '5', ';', '3', '7',
                                       ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_WHITE_WHITE[] = {0x1b, '[', '5', ';', '3', '7',
                                        ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_WHITE_NORMAL[] = {0x1b, '[', '5', ';', '3', '7',
                                         ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_NORMAL_BLACK[] = {0x1b, '[', '5', ';', '3', '9',
                                         ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_NORMAL_RED[] = {0x1b, '[', '5', ';', '3', '9',
                                       ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_NORMAL_GREEN[] = {0x1b, '[', '5', ';', '3', '9',
                                         ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_NORMAL_YELLOW[] = {0x1b, '[', '5', ';', '3', '9',
                                          ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_NORMAL_BLUE[] = {0x1b, '[', '5', ';', '3', '9',
                                        ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_NORMAL_MAGENTA[] = {0x1b, '[', '5', ';', '3', '9',
                                           ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_NORMAL_CYAN[] = {0x1b, '[', '5', ';', '3', '9',
                                        ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_NORMAL_WHITE[] = {0x1b, '[', '5', ';', '3', '9',
                                         ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_NORMAL_NORMAL[] = {0x1b, '[', '5', ';', '3', '9',
                                          ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_BLACK[] = {0x1b, '[', '7', ';', '3', '0',
                                               ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_RED[] = {0x1b, '[', '7', ';', '3', '0',
                                             ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_GREEN[] = {0x1b, '[', '7', ';', '3', '0',
                                               ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_YELLOW[] = {0x1b, '[', '7', ';', '3', '0',
                                                ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_BLUE[] = {0x1b, '[', '7', ';', '3', '0',
                                              ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_MAGENTA[] = {0x1b, '[', '7', ';', '3', '0',
                                                 ';',  '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_CYAN[] = {0x1b, '[', '7', ';', '3', '0',
                                              ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_WHITE[] = {0x1b, '[', '7', ';', '3', '0',
                                               ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_NORMAL[] = {0x1b, '[', '7', ';', '3', '0',
                                                ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_BLACK[] = {0x1b, '[', '7', ';', '3', '1',
                                             ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_RED[] = {0x1b, '[', '7', ';', '3', '1',
                                           ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_GREEN[] = {0x1b, '[', '7', ';', '3', '1',
                                             ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_YELLOW[] = {0x1b, '[', '7', ';', '3', '1',
                                              ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_BLUE[] = {0x1b, '[', '7', ';', '3', '1',
                                            ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_MAGENTA[] = {0x1b, '[', '7', ';', '3', '1',
                                               ';',  '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_CYAN[] = {0x1b, '[', '7', ';', '3', '1',
                                            ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_WHITE[] = {0x1b, '[', '7', ';', '3', '1',
                                             ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_NORMAL[] = {0x1b, '[', '7', ';', '3', '1',
                                              ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_BLACK[] = {0x1b, '[', '7', ';', '3', '2',
                                               ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_RED[] = {0x1b, '[', '7', ';', '3', '2',
                                             ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_GREEN[] = {0x1b, '[', '7', ';', '3', '2',
                                               ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_YELLOW[] = {0x1b, '[', '7', ';', '3', '2',
                                                ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_BLUE[] = {0x1b, '[', '7', ';', '3', '2',
                                              ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_MAGENTA[] = {0x1b, '[', '7', ';', '3', '2',
                                                 ';',  '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_CYAN[] = {0x1b, '[', '7', ';', '3', '2',
                                              ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_WHITE[] = {0x1b, '[', '7', ';', '3', '2',
                                               ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_NORMAL[] = {0x1b, '[', '7', ';', '3', '2',
                                                ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_BLACK[] = {0x1b, '[', '7', ';', '3', '3',
                                                ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_RED[] = {0x1b, '[', '7', ';', '3', '3',
                                              ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_GREEN[] = {0x1b, '[', '7', ';', '3', '3',
                                                ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_YELLOW[] = {0x1b, '[', '7', ';', '3', '3',
                                                 ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_BLUE[] = {0x1b, '[', '7', ';', '3', '3',
                                               ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_MAGENTA[] = {0x1b, '[', '7', ';', '3', '3',
                                                  ';',  '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_CYAN[] = {0x1b, '[', '7', ';', '3', '3',
                                               ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_WHITE[] = {0x1b, '[', '7', ';', '3', '3',
                                                ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_NORMAL[] = {0x1b, '[', '7', ';', '3', '3',
                                                 ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_BLACK[] = {0x1b, '[', '7', ';', '3', '4',
                                              ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_RED[] = {0x1b, '[', '7', ';', '3', '4',
                                            ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_GREEN[] = {0x1b, '[', '7', ';', '3', '4',
                                              ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_YELLOW[] = {0x1b, '[', '7', ';', '3', '4',
                                               ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_BLUE[] = {0x1b, '[', '7', ';', '3', '4',
                                             ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_MAGENTA[] = {0x1b, '[', '7', ';', '3', '4',
                                                ';',  '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_CYAN[] = {0x1b, '[', '7', ';', '3', '4',
                                             ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_WHITE[] = {0x1b, '[', '7', ';', '3', '4',
                                              ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_NORMAL[] = {0x1b, '[', '7', ';', '3', '4',
                                               ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_BLACK[] = {0x1b, '[', '7', ';', '3', '5',
                                                 ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_RED[] = {0x1b, '[', '7', ';', '3', '5',
                                               ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_GREEN[] = {0x1b, '[', '7', ';', '3', '5',
                                                 ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_YELLOW[] = {0x1b, '[', '7', ';', '3', '5',
                                                  ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_BLUE[] = {0x1b, '[', '7', ';', '3', '5',
                                                ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_MAGENTA[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_CYAN[] = {0x1b, '[', '7', ';', '3', '5',
                                                ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_WHITE[] = {0x1b, '[', '7', ';', '3', '5',
                                                 ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_NORMAL[] = {0x1b, '[', '7', ';', '3', '5',
                                                  ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_BLACK[] = {0x1b, '[', '7', ';', '3', '6',
                                              ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_RED[] = {0x1b, '[', '7', ';', '3', '6',
                                            ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_GREEN[] = {0x1b, '[', '7', ';', '3', '6',
                                              ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_YELLOW[] = {0x1b, '[', '7', ';', '3', '6',
                                               ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_BLUE[] = {0x1b, '[', '7', ';', '3', '6',
                                             ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_MAGENTA[] = {0x1b, '[', '7', ';', '3', '6',
                                                ';',  '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_CYAN[] = {0x1b, '[', '7', ';', '3', '6',
                                             ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_WHITE[] = {0x1b, '[', '7', ';', '3', '6',
                                              ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_NORMAL[] = {0x1b, '[', '7', ';', '3', '6',
                                               ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_BLACK[] = {0x1b, '[', '7', ';', '3', '7',
                                               ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_RED[] = {0x1b, '[', '7', ';', '3', '7',
                                             ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_GREEN[] = {0x1b, '[', '7', ';', '3', '7',
                                               ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_YELLOW[] = {0x1b, '[', '7', ';', '3', '7',
                                                ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_BLUE[] = {0x1b, '[', '7', ';', '3', '7',
                                              ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_MAGENTA[] = {0x1b, '[', '7', ';', '3', '7',
                                                 ';',  '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_CYAN[] = {0x1b, '[', '7', ';', '3', '7',
                                              ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_WHITE[] = {0x1b, '[', '7', ';', '3', '7',
                                               ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_NORMAL[] = {0x1b, '[', '7', ';', '3', '7',
                                                ';',  '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_BLACK[] = {0x1b, '[', '7', ';', '3', '9',
                                                ';',  '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_RED[] = {0x1b, '[', '7', ';', '3', '9',
                                              ';',  '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_GREEN[] = {0x1b, '[', '7', ';', '3', '9',
                                                ';',  '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_YELLOW[] = {0x1b, '[', '7', ';', '3', '9',
                                                 ';',  '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_BLUE[] = {0x1b, '[', '7', ';', '3', '9',
                                               ';',  '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_MAGENTA[] = {0x1b, '[', '7', ';', '3', '9',
                                                  ';',  '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_CYAN[] = {0x1b, '[', '7', ';', '3', '9',
                                               ';',  '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_WHITE[] = {0x1b, '[', '7', ';', '3', '9',
                                                ';',  '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_NORMAL[] = {0x1b, '[', '7', ';', '3', '9',
                                                 ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_BLACK_BLACK[] = {0x1b, '[', '8', ';', '3', '0',
                                            ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_BLACK_RED[] = {0x1b, '[', '8', ';', '3', '0',
                                          ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_BLACK_GREEN[] = {0x1b, '[', '8', ';', '3', '0',
                                            ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_BLACK_YELLOW[] = {0x1b, '[', '8', ';', '3', '0',
                                             ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_BLACK_BLUE[] = {0x1b, '[', '8', ';', '3', '0',
                                           ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_BLACK_MAGENTA[] = {0x1b, '[', '8', ';', '3', '0',
                                              ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_BLACK_CYAN[] = {0x1b, '[', '8', ';', '3', '0',
                                           ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_BLACK_WHITE[] = {0x1b, '[', '8', ';', '3', '0',
                                            ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_BLACK_NORMAL[] = {0x1b, '[', '8', ';', '3', '0',
                                             ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_RED_BLACK[] = {0x1b, '[', '8', ';', '3', '1',
                                          ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_RED_RED[] = {0x1b, '[', '8', ';', '3', '1',
                                        ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_RED_GREEN[] = {0x1b, '[', '8', ';', '3', '1',
                                          ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_RED_YELLOW[] = {0x1b, '[', '8', ';', '3', '1',
                                           ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_RED_BLUE[] = {0x1b, '[', '8', ';', '3', '1',
                                         ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_RED_MAGENTA[] = {0x1b, '[', '8', ';', '3', '1',
                                            ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_RED_CYAN[] = {0x1b, '[', '8', ';', '3', '1',
                                         ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_RED_WHITE[] = {0x1b, '[', '8', ';', '3', '1',
                                          ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_RED_NORMAL[] = {0x1b, '[', '8', ';', '3', '1',
                                           ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_GREEN_BLACK[] = {0x1b, '[', '8', ';', '3', '2',
                                            ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_GREEN_RED[] = {0x1b, '[', '8', ';', '3', '2',
                                          ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_GREEN_GREEN[] = {0x1b, '[', '8', ';', '3', '2',
                                            ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_GREEN_YELLOW[] = {0x1b, '[', '8', ';', '3', '2',
                                             ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_GREEN_BLUE[] = {0x1b, '[', '8', ';', '3', '2',
                                           ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_GREEN_MAGENTA[] = {0x1b, '[', '8', ';', '3', '2',
                                              ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_GREEN_CYAN[] = {0x1b, '[', '8', ';', '3', '2',
                                           ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_GREEN_WHITE[] = {0x1b, '[', '8', ';', '3', '2',
                                            ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_GREEN_NORMAL[] = {0x1b, '[', '8', ';', '3', '2',
                                             ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_YELLOW_BLACK[] = {0x1b, '[', '8', ';', '3', '3',
                                             ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_YELLOW_RED[] = {0x1b, '[', '8', ';', '3', '3',
                                           ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_YELLOW_GREEN[] = {0x1b, '[', '8', ';', '3', '3',
                                             ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_YELLOW_YELLOW[] = {0x1b, '[', '8', ';', '3', '3',
                                              ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_YELLOW_BLUE[] = {0x1b, '[', '8', ';', '3', '3',
                                            ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_YELLOW_MAGENTA[] = {0x1b, '[', '8', ';', '3', '3',
                                               ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_YELLOW_CYAN[] = {0x1b, '[', '8', ';', '3', '3',
                                            ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_YELLOW_WHITE[] = {0x1b, '[', '8', ';', '3', '3',
                                             ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_YELLOW_NORMAL[] = {0x1b, '[', '8', ';', '3', '3',
                                              ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_BLUE_BLACK[] = {0x1b, '[', '8', ';', '3', '4',
                                           ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_BLUE_RED[] = {0x1b, '[', '8', ';', '3', '4',
                                         ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_BLUE_GREEN[] = {0x1b, '[', '8', ';', '3', '4',
                                           ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_BLUE_YELLOW[] = {0x1b, '[', '8', ';', '3', '4',
                                            ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_BLUE_BLUE[] = {0x1b, '[', '8', ';', '3', '4',
                                          ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_BLUE_MAGENTA[] = {0x1b, '[', '8', ';', '3', '4',
                                             ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_BLUE_CYAN[] = {0x1b, '[', '8', ';', '3', '4',
                                          ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_BLUE_WHITE[] = {0x1b, '[', '8', ';', '3', '4',
                                           ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_BLUE_NORMAL[] = {0x1b, '[', '8', ';', '3', '4',
                                            ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_BLACK[] = {0x1b, '[', '8', ';', '3', '5',
                                              ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_RED[] = {0x1b, '[', '8', ';', '3', '5',
                                            ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_GREEN[] = {0x1b, '[', '8', ';', '3', '5',
                                              ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_YELLOW[] = {0x1b, '[', '8', ';', '3', '5',
                                               ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_BLUE[] = {0x1b, '[', '8', ';', '3', '5',
                                             ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_MAGENTA[] = {0x1b, '[', '8', ';', '3', '5',
                                                ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_CYAN[] = {0x1b, '[', '8', ';', '3', '5',
                                             ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_WHITE[] = {0x1b, '[', '8', ';', '3', '5',
                                              ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_NORMAL[] = {0x1b, '[', '8', ';', '3', '5',
                                               ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_CYAN_BLACK[] = {0x1b, '[', '8', ';', '3', '6',
                                           ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_CYAN_RED[] = {0x1b, '[', '8', ';', '3', '6',
                                         ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_CYAN_GREEN[] = {0x1b, '[', '8', ';', '3', '6',
                                           ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_CYAN_YELLOW[] = {0x1b, '[', '8', ';', '3', '6',
                                            ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_CYAN_BLUE[] = {0x1b, '[', '8', ';', '3', '6',
                                          ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_CYAN_MAGENTA[] = {0x1b, '[', '8', ';', '3', '6',
                                             ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_CYAN_CYAN[] = {0x1b, '[', '8', ';', '3', '6',
                                          ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_CYAN_WHITE[] = {0x1b, '[', '8', ';', '3', '6',
                                           ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_CYAN_NORMAL[] = {0x1b, '[', '8', ';', '3', '6',
                                            ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_WHITE_BLACK[] = {0x1b, '[', '8', ';', '3', '7',
                                            ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_WHITE_RED[] = {0x1b, '[', '8', ';', '3', '7',
                                          ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_WHITE_GREEN[] = {0x1b, '[', '8', ';', '3', '7',
                                            ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_WHITE_YELLOW[] = {0x1b, '[', '8', ';', '3', '7',
                                             ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_WHITE_BLUE[] = {0x1b, '[', '8', ';', '3', '7',
                                           ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_WHITE_MAGENTA[] = {0x1b, '[', '8', ';', '3', '7',
                                              ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_WHITE_CYAN[] = {0x1b, '[', '8', ';', '3', '7',
                                           ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_WHITE_WHITE[] = {0x1b, '[', '8', ';', '3', '7',
                                            ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_WHITE_NORMAL[] = {0x1b, '[', '8', ';', '3', '7',
                                             ';',  '4', '9', 'm', 0};
const char COLOR_CONCEALED_NORMAL_BLACK[] = {0x1b, '[', '8', ';', '3', '9',
                                             ';',  '4', '0', 'm', 0};
const char COLOR_CONCEALED_NORMAL_RED[] = {0x1b, '[', '8', ';', '3', '9',
                                           ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_NORMAL_GREEN[] = {0x1b, '[', '8', ';', '3', '9',
                                             ';',  '4', '2', 'm', 0};
const char COLOR_CONCEALED_NORMAL_YELLOW[] = {0x1b, '[', '8', ';', '3', '9',
                                              ';',  '4', '3', 'm', 0};
const char COLOR_CONCEALED_NORMAL_BLUE[] = {0x1b, '[', '8', ';', '3', '9',
                                            ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_NORMAL_MAGENTA[] = {0x1b, '[', '8', ';', '3', '9',
                                               ';',  '4', '5', 'm', 0};
const char COLOR_CONCEALED_NORMAL_CYAN[] = {0x1b, '[', '8', ';', '3', '9',
                                            ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_NORMAL_WHITE[] = {0x1b, '[', '8', ';', '3', '9',
                                             ';',  '4', '7', 'm', 0};
const char COLOR_CONCEALED_NORMAL_NORMAL[] = {0x1b, '[', '8', ';', '3', '9',
                                              ';',  '4', '9', 'm', 0};
const char COLOR_ATTRIBUTES_OFF_COLOREND[] = {0x1b, '[', '0', 'm', 0};
const char COLOR_BOLD_COLOREND[] = {0x1b, '[', '1', 'm', 0};
const char COLOR_UNDERSCORE_COLOREND[] = {0x1b, '[', '4', 'm', 0};
const char COLOR_BLINK_COLOREND[] = {0x1b, '[', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_COLOREND[] = {0x1b, '[', '7', 'm', 0};
const char COLOR_CONCEALED_COLOREND[] = {0x1b, '[', '8', 'm', 0};
const char COLOR_BLACK_BLACK_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                           '4',  '0', 'm', 0};
const char COLOR_BLACK_RED_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                         '4',  '1', 'm', 0};
const char COLOR_BLACK_GREEN_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                           '4',  '2', 'm', 0};
const char COLOR_BLACK_YELLOW_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                            '4',  '3', 'm', 0};
const char COLOR_BLACK_BLUE_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                          '4',  '4', 'm', 0};
const char COLOR_BLACK_MAGENTA_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                             '4',  '5', 'm', 0};
const char COLOR_BLACK_CYAN_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                          '4',  '6', 'm', 0};
const char COLOR_BLACK_WHITE_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                           '4',  '7', 'm', 0};
const char COLOR_BLACK_NORMAL_COLOREND[] = {0x1b, '[', '3', '0', ';',
                                            '4',  '9', 'm', 0};
const char COLOR_RED_BLACK_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                         '4',  '0', 'm', 0};
const char COLOR_RED_RED_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                       '4',  '1', 'm', 0};
const char COLOR_RED_GREEN_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                         '4',  '2', 'm', 0};
const char COLOR_RED_YELLOW_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                          '4',  '3', 'm', 0};
const char COLOR_RED_BLUE_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                        '4',  '4', 'm', 0};
const char COLOR_RED_MAGENTA_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                           '4',  '5', 'm', 0};
const char COLOR_RED_CYAN_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                        '4',  '6', 'm', 0};
const char COLOR_RED_WHITE_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                         '4',  '7', 'm', 0};
const char COLOR_RED_NORMAL_COLOREND[] = {0x1b, '[', '3', '1', ';',
                                          '4',  '9', 'm', 0};
const char COLOR_GREEN_BLACK_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                           '4',  '0', 'm', 0};
const char COLOR_GREEN_RED_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                         '4',  '1', 'm', 0};
const char COLOR_GREEN_GREEN_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                           '4',  '2', 'm', 0};
const char COLOR_GREEN_YELLOW_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                            '4',  '3', 'm', 0};
const char COLOR_GREEN_BLUE_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                          '4',  '4', 'm', 0};
const char COLOR_GREEN_MAGENTA_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                             '4',  '5', 'm', 0};
const char COLOR_GREEN_CYAN_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                          '4',  '6', 'm', 0};
const char COLOR_GREEN_WHITE_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                           '4',  '7', 'm', 0};
const char COLOR_GREEN_NORMAL_COLOREND[] = {0x1b, '[', '3', '2', ';',
                                            '4',  '9', 'm', 0};
const char COLOR_YELLOW_BLACK_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                            '4',  '0', 'm', 0};
const char COLOR_YELLOW_RED_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                          '4',  '1', 'm', 0};
const char COLOR_YELLOW_GREEN_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                            '4',  '2', 'm', 0};
const char COLOR_YELLOW_YELLOW_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                             '4',  '3', 'm', 0};
const char COLOR_YELLOW_BLUE_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                           '4',  '4', 'm', 0};
const char COLOR_YELLOW_MAGENTA_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                              '4',  '5', 'm', 0};
const char COLOR_YELLOW_CYAN_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                           '4',  '6', 'm', 0};
const char COLOR_YELLOW_WHITE_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                            '4',  '7', 'm', 0};
const char COLOR_YELLOW_NORMAL_COLOREND[] = {0x1b, '[', '3', '3', ';',
                                             '4',  '9', 'm', 0};
const char COLOR_BLUE_BLACK_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                          '4',  '0', 'm', 0};
const char COLOR_BLUE_RED_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                        '4',  '1', 'm', 0};
const char COLOR_BLUE_GREEN_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                          '4',  '2', 'm', 0};
const char COLOR_BLUE_YELLOW_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                           '4',  '3', 'm', 0};
const char COLOR_BLUE_BLUE_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                         '4',  '4', 'm', 0};
const char COLOR_BLUE_MAGENTA_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                            '4',  '5', 'm', 0};
const char COLOR_BLUE_CYAN_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                         '4',  '6', 'm', 0};
const char COLOR_BLUE_WHITE_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                          '4',  '7', 'm', 0};
const char COLOR_BLUE_NORMAL_COLOREND[] = {0x1b, '[', '3', '4', ';',
                                           '4',  '9', 'm', 0};
const char COLOR_MAGENTA_BLACK_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                             '4',  '0', 'm', 0};
const char COLOR_MAGENTA_RED_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                           '4',  '1', 'm', 0};
const char COLOR_MAGENTA_GREEN_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                             '4',  '2', 'm', 0};
const char COLOR_MAGENTA_YELLOW_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                              '4',  '3', 'm', 0};
const char COLOR_MAGENTA_BLUE_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                            '4',  '4', 'm', 0};
const char COLOR_MAGENTA_MAGENTA_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                               '4',  '5', 'm', 0};
const char COLOR_MAGENTA_CYAN_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                            '4',  '6', 'm', 0};
const char COLOR_MAGENTA_WHITE_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                             '4',  '7', 'm', 0};
const char COLOR_MAGENTA_NORMAL_COLOREND[] = {0x1b, '[', '3', '5', ';',
                                              '4',  '9', 'm', 0};
const char COLOR_CYAN_BLACK_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                          '4',  '0', 'm', 0};
const char COLOR_CYAN_RED_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                        '4',  '1', 'm', 0};
const char COLOR_CYAN_GREEN_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                          '4',  '2', 'm', 0};
const char COLOR_CYAN_YELLOW_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                           '4',  '3', 'm', 0};
const char COLOR_CYAN_BLUE_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                         '4',  '4', 'm', 0};
const char COLOR_CYAN_MAGENTA_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                            '4',  '5', 'm', 0};
const char COLOR_CYAN_CYAN_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                         '4',  '6', 'm', 0};
const char COLOR_CYAN_WHITE_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                          '4',  '7', 'm', 0};
const char COLOR_CYAN_NORMAL_COLOREND[] = {0x1b, '[', '3', '6', ';',
                                           '4',  '9', 'm', 0};
const char COLOR_WHITE_BLACK_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                           '4',  '0', 'm', 0};
const char COLOR_WHITE_RED_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                         '4',  '1', 'm', 0};
const char COLOR_WHITE_GREEN_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                           '4',  '2', 'm', 0};
const char COLOR_WHITE_YELLOW_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                            '4',  '3', 'm', 0};
const char COLOR_WHITE_BLUE_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                          '4',  '4', 'm', 0};
const char COLOR_WHITE_MAGENTA_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                             '4',  '5', 'm', 0};
const char COLOR_WHITE_CYAN_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                          '4',  '6', 'm', 0};
const char COLOR_WHITE_WHITE_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                           '4',  '7', 'm', 0};
const char COLOR_WHITE_NORMAL_COLOREND[] = {0x1b, '[', '3', '7', ';',
                                            '4',  '9', 'm', 0};
const char COLOR_NORMAL_BLACK_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                            '4',  '0', 'm', 0};
const char COLOR_NORMAL_RED_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                          '4',  '1', 'm', 0};
const char COLOR_NORMAL_GREEN_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                            '4',  '2', 'm', 0};
const char COLOR_NORMAL_YELLOW_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                             '4',  '3', 'm', 0};
const char COLOR_NORMAL_BLUE_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                           '4',  '4', 'm', 0};
const char COLOR_NORMAL_MAGENTA_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                              '4',  '5', 'm', 0};
const char COLOR_NORMAL_CYAN_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                           '4',  '6', 'm', 0};
const char COLOR_NORMAL_WHITE_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                            '4',  '7', 'm', 0};
const char COLOR_NORMAL_NORMAL_COLOREND[] = {0x1b, '[', '3', '9', ';',
                                             '4',  '9', 'm', 0};
const char COLOR_BOLD_BLACK_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                                ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_BLACK_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                              ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_BLACK_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                                ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_BLACK_YELLOW_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                                 ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_BLACK_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                               ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_BLACK_MAGENTA_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                                  ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_BLACK_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                               ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_BLACK_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                                ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_BLACK_NORMAL_COLOREND[] = {0x1b, '[', '1', ';', '3', '0',
                                                 ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_RED_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                              ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_RED_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                            ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_RED_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                              ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_RED_YELLOW_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                               ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_RED_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                             ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_RED_MAGENTA_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                                ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_RED_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                             ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_RED_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                              ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_RED_NORMAL_COLOREND[] = {0x1b, '[', '1', ';', '3', '1',
                                               ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_GREEN_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                                ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_GREEN_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                              ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_GREEN_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                                ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_GREEN_YELLOW_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                                 ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_GREEN_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                               ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_GREEN_MAGENTA_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                                  ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_GREEN_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                               ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_GREEN_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                                ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_GREEN_NORMAL_COLOREND[] = {0x1b, '[', '1', ';', '3', '2',
                                                 ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_YELLOW_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '3',
                                                 ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_YELLOW_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '3',
                                               ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_YELLOW_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '3',
                                                 ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_YELLOW_YELLOW_COLOREND[] = {0x1b, '[', '1', ';', '3', '3',
                                                  ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_YELLOW_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '3',
                                                ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_YELLOW_MAGENTA_COLOREND[] = {
    0x1b, '[', '1', ';', '3', '3', ';', '4', '5', 'm', 0};
const char COLOR_BOLD_YELLOW_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '3',
                                                ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_YELLOW_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '3',
                                                 ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_YELLOW_NORMAL_COLOREND[] = {0x1b, '[', '1', ';', '3', '3',
                                                  ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_BLUE_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                               ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_BLUE_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                             ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_BLUE_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                               ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_BLUE_YELLOW_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                                ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_BLUE_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                              ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_BLUE_MAGENTA_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                                 ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_BLUE_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                              ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_BLUE_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                               ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_BLUE_NORMAL_COLOREND[] = {0x1b, '[', '1', ';', '3', '4',
                                                ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_MAGENTA_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '5',
                                                  ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_MAGENTA_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '5',
                                                ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_MAGENTA_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '5',
                                                  ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_MAGENTA_YELLOW_COLOREND[] = {
    0x1b, '[', '1', ';', '3', '5', ';', '4', '3', 'm', 0};
const char COLOR_BOLD_MAGENTA_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '5',
                                                 ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_MAGENTA_MAGENTA_COLOREND[] = {
    0x1b, '[', '1', ';', '3', '5', ';', '4', '5', 'm', 0};
const char COLOR_BOLD_MAGENTA_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '5',
                                                 ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_MAGENTA_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '5',
                                                  ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_MAGENTA_NORMAL_COLOREND[] = {
    0x1b, '[', '1', ';', '3', '5', ';', '4', '9', 'm', 0};
const char COLOR_BOLD_CYAN_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                               ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_CYAN_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                             ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_CYAN_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                               ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_CYAN_YELLOW_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                                ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_CYAN_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                              ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_CYAN_MAGENTA_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                                 ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_CYAN_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                              ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_CYAN_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                               ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_CYAN_NORMAL_COLOREND[] = {0x1b, '[', '1', ';', '3', '6',
                                                ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_WHITE_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                                ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_WHITE_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                              ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_WHITE_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                                ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_WHITE_YELLOW_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                                 ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_WHITE_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                               ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_WHITE_MAGENTA_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                                  ';',  '4', '5', 'm', 0};
const char COLOR_BOLD_WHITE_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                               ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_WHITE_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                                ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_WHITE_NORMAL_COLOREND[] = {0x1b, '[', '1', ';', '3', '7',
                                                 ';',  '4', '9', 'm', 0};
const char COLOR_BOLD_NORMAL_BLACK_COLOREND[] = {0x1b, '[', '1', ';', '3', '9',
                                                 ';',  '4', '0', 'm', 0};
const char COLOR_BOLD_NORMAL_RED_COLOREND[] = {0x1b, '[', '1', ';', '3', '9',
                                               ';',  '4', '1', 'm', 0};
const char COLOR_BOLD_NORMAL_GREEN_COLOREND[] = {0x1b, '[', '1', ';', '3', '9',
                                                 ';',  '4', '2', 'm', 0};
const char COLOR_BOLD_NORMAL_YELLOW_COLOREND[] = {0x1b, '[', '1', ';', '3', '9',
                                                  ';',  '4', '3', 'm', 0};
const char COLOR_BOLD_NORMAL_BLUE_COLOREND[] = {0x1b, '[', '1', ';', '3', '9',
                                                ';',  '4', '4', 'm', 0};
const char COLOR_BOLD_NORMAL_MAGENTA_COLOREND[] = {
    0x1b, '[', '1', ';', '3', '9', ';', '4', '5', 'm', 0};
const char COLOR_BOLD_NORMAL_CYAN_COLOREND[] = {0x1b, '[', '1', ';', '3', '9',
                                                ';',  '4', '6', 'm', 0};
const char COLOR_BOLD_NORMAL_WHITE_COLOREND[] = {0x1b, '[', '1', ';', '3', '9',
                                                 ';',  '4', '7', 'm', 0};
const char COLOR_BOLD_NORMAL_NORMAL_COLOREND[] = {0x1b, '[', '1', ';', '3', '9',
                                                  ';',  '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_RED_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_BLACK_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '0', ';', '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_RED_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '1', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_RED_RED_COLOREND[] = {0x1b, '[', '4', ';', '3', '1',
                                                  ';',  '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_RED_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '1', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_RED_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '1', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_RED_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '1', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_RED_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '1', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_RED_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '1', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_RED_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '1', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_RED_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '1', ';', '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_RED_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_GREEN_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '2', ';', '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_RED_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_YELLOW_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '3', ';', '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_RED_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_BLUE_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '4', ';', '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_RED_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_MAGENTA_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '5', ';', '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_RED_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_CYAN_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '6', ';', '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_RED_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_WHITE_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '7', ';', '4', '9', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_BLACK_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '0', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_RED_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '1', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_GREEN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '2', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_YELLOW_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '3', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_BLUE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '4', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_MAGENTA_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '5', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_CYAN_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '6', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_WHITE_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '7', 'm', 0};
const char COLOR_UNDERSCORE_NORMAL_NORMAL_COLOREND[] = {
    0x1b, '[', '4', ';', '3', '9', ';', '4', '9', 'm', 0};
const char COLOR_BLINK_BLACK_BLACK_COLOREND[] = {0x1b, '[', '5', ';', '3', '0',
                                                 ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_BLACK_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '0',
                                               ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_BLACK_GREEN_COLOREND[] = {0x1b, '[', '5', ';', '3', '0',
                                                 ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_BLACK_YELLOW_COLOREND[] = {0x1b, '[', '5', ';', '3', '0',
                                                  ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_BLACK_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '0',
                                                ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_BLACK_MAGENTA_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '0', ';', '4', '5', 'm', 0};
const char COLOR_BLINK_BLACK_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '0',
                                                ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_BLACK_WHITE_COLOREND[] = {0x1b, '[', '5', ';', '3', '0',
                                                 ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_BLACK_NORMAL_COLOREND[] = {0x1b, '[', '5', ';', '3', '0',
                                                  ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_RED_BLACK_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                               ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_RED_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                             ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_RED_GREEN_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                               ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_RED_YELLOW_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                                ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_RED_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                              ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_RED_MAGENTA_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                                 ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_RED_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                              ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_RED_WHITE_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                               ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_RED_NORMAL_COLOREND[] = {0x1b, '[', '5', ';', '3', '1',
                                                ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_GREEN_BLACK_COLOREND[] = {0x1b, '[', '5', ';', '3', '2',
                                                 ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_GREEN_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '2',
                                               ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_GREEN_GREEN_COLOREND[] = {0x1b, '[', '5', ';', '3', '2',
                                                 ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_GREEN_YELLOW_COLOREND[] = {0x1b, '[', '5', ';', '3', '2',
                                                  ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_GREEN_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '2',
                                                ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_GREEN_MAGENTA_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '2', ';', '4', '5', 'm', 0};
const char COLOR_BLINK_GREEN_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '2',
                                                ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_GREEN_WHITE_COLOREND[] = {0x1b, '[', '5', ';', '3', '2',
                                                 ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_GREEN_NORMAL_COLOREND[] = {0x1b, '[', '5', ';', '3', '2',
                                                  ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_YELLOW_BLACK_COLOREND[] = {0x1b, '[', '5', ';', '3', '3',
                                                  ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_YELLOW_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '3',
                                                ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_YELLOW_GREEN_COLOREND[] = {0x1b, '[', '5', ';', '3', '3',
                                                  ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_YELLOW_YELLOW_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '3', ';', '4', '3', 'm', 0};
const char COLOR_BLINK_YELLOW_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '3',
                                                 ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_YELLOW_MAGENTA_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '3', ';', '4', '5', 'm', 0};
const char COLOR_BLINK_YELLOW_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '3',
                                                 ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_YELLOW_WHITE_COLOREND[] = {0x1b, '[', '5', ';', '3', '3',
                                                  ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_YELLOW_NORMAL_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '3', ';', '4', '9', 'm', 0};
const char COLOR_BLINK_BLUE_BLACK_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                                ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_BLUE_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                              ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_BLUE_GREEN_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                                ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_BLUE_YELLOW_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                                 ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_BLUE_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                               ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_BLUE_MAGENTA_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                                  ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_BLUE_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                               ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_BLUE_WHITE_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                                ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_BLUE_NORMAL_COLOREND[] = {0x1b, '[', '5', ';', '3', '4',
                                                 ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_MAGENTA_BLACK_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '5', ';', '4', '0', 'm', 0};
const char COLOR_BLINK_MAGENTA_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '5',
                                                 ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_MAGENTA_GREEN_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '5', ';', '4', '2', 'm', 0};
const char COLOR_BLINK_MAGENTA_YELLOW_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '5', ';', '4', '3', 'm', 0};
const char COLOR_BLINK_MAGENTA_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '5',
                                                  ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_MAGENTA_MAGENTA_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '5', ';', '4', '5', 'm', 0};
const char COLOR_BLINK_MAGENTA_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '5',
                                                  ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_MAGENTA_WHITE_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '5', ';', '4', '7', 'm', 0};
const char COLOR_BLINK_MAGENTA_NORMAL_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '5', ';', '4', '9', 'm', 0};
const char COLOR_BLINK_CYAN_BLACK_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                                ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_CYAN_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                              ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_CYAN_GREEN_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                                ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_CYAN_YELLOW_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                                 ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_CYAN_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                               ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_CYAN_MAGENTA_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                                  ';',  '4', '5', 'm', 0};
const char COLOR_BLINK_CYAN_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                               ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_CYAN_WHITE_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                                ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_CYAN_NORMAL_COLOREND[] = {0x1b, '[', '5', ';', '3', '6',
                                                 ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_WHITE_BLACK_COLOREND[] = {0x1b, '[', '5', ';', '3', '7',
                                                 ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_WHITE_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '7',
                                               ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_WHITE_GREEN_COLOREND[] = {0x1b, '[', '5', ';', '3', '7',
                                                 ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_WHITE_YELLOW_COLOREND[] = {0x1b, '[', '5', ';', '3', '7',
                                                  ';',  '4', '3', 'm', 0};
const char COLOR_BLINK_WHITE_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '7',
                                                ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_WHITE_MAGENTA_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '7', ';', '4', '5', 'm', 0};
const char COLOR_BLINK_WHITE_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '7',
                                                ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_WHITE_WHITE_COLOREND[] = {0x1b, '[', '5', ';', '3', '7',
                                                 ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_WHITE_NORMAL_COLOREND[] = {0x1b, '[', '5', ';', '3', '7',
                                                  ';',  '4', '9', 'm', 0};
const char COLOR_BLINK_NORMAL_BLACK_COLOREND[] = {0x1b, '[', '5', ';', '3', '9',
                                                  ';',  '4', '0', 'm', 0};
const char COLOR_BLINK_NORMAL_RED_COLOREND[] = {0x1b, '[', '5', ';', '3', '9',
                                                ';',  '4', '1', 'm', 0};
const char COLOR_BLINK_NORMAL_GREEN_COLOREND[] = {0x1b, '[', '5', ';', '3', '9',
                                                  ';',  '4', '2', 'm', 0};
const char COLOR_BLINK_NORMAL_YELLOW_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '9', ';', '4', '3', 'm', 0};
const char COLOR_BLINK_NORMAL_BLUE_COLOREND[] = {0x1b, '[', '5', ';', '3', '9',
                                                 ';',  '4', '4', 'm', 0};
const char COLOR_BLINK_NORMAL_MAGENTA_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '9', ';', '4', '5', 'm', 0};
const char COLOR_BLINK_NORMAL_CYAN_COLOREND[] = {0x1b, '[', '5', ';', '3', '9',
                                                 ';',  '4', '6', 'm', 0};
const char COLOR_BLINK_NORMAL_WHITE_COLOREND[] = {0x1b, '[', '5', ';', '3', '9',
                                                  ';',  '4', '7', 'm', 0};
const char COLOR_BLINK_NORMAL_NORMAL_COLOREND[] = {
    0x1b, '[', '5', ';', '3', '9', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_BLACK_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '0', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_RED_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '1', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_GREEN_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '2', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_YELLOW_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '3', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_BLUE_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '4', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_MAGENTA_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '5', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_CYAN_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '6', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_WHITE_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '7', ';', '4', '9', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_BLACK_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '0', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_RED_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '1', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_GREEN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '2', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_YELLOW_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '3', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_BLUE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '4', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_MAGENTA_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '5', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_CYAN_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '6', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_WHITE_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '7', 'm', 0};
const char COLOR_REVERSEVIDEO_NORMAL_NORMAL_COLOREND[] = {
    0x1b, '[', '7', ';', '3', '9', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_BLACK_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_BLACK_RED_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '1', 'm', 0};
const char COLOR_CONCEALED_BLACK_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_BLACK_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_BLACK_BLUE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '4', 'm', 0};
const char COLOR_CONCEALED_BLACK_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_BLACK_CYAN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '6', 'm', 0};
const char COLOR_CONCEALED_BLACK_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_BLACK_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '0', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_RED_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '1', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_RED_RED_COLOREND[] = {0x1b, '[', '8', ';', '3', '1',
                                                 ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_RED_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '1', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_RED_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '1', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_RED_BLUE_COLOREND[] = {0x1b, '[', '8', ';', '3', '1',
                                                  ';',  '4', '4', 'm', 0};
const char COLOR_CONCEALED_RED_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '1', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_RED_CYAN_COLOREND[] = {0x1b, '[', '8', ';', '3', '1',
                                                  ';',  '4', '6', 'm', 0};
const char COLOR_CONCEALED_RED_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '1', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_RED_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '1', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_GREEN_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_GREEN_RED_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '1', 'm', 0};
const char COLOR_CONCEALED_GREEN_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_GREEN_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_GREEN_BLUE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '4', 'm', 0};
const char COLOR_CONCEALED_GREEN_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_GREEN_CYAN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '6', 'm', 0};
const char COLOR_CONCEALED_GREEN_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_GREEN_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '2', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_YELLOW_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_YELLOW_RED_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '1', 'm', 0};
const char COLOR_CONCEALED_YELLOW_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_YELLOW_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_YELLOW_BLUE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '4', 'm', 0};
const char COLOR_CONCEALED_YELLOW_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_YELLOW_CYAN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '6', 'm', 0};
const char COLOR_CONCEALED_YELLOW_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_YELLOW_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '3', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_BLUE_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '4', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_BLUE_RED_COLOREND[] = {0x1b, '[', '8', ';', '3', '4',
                                                  ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_BLUE_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '4', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_BLUE_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '4', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_BLUE_BLUE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '4', ';', '4', '4', 'm', 0};
const char COLOR_CONCEALED_BLUE_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '4', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_BLUE_CYAN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '4', ';', '4', '6', 'm', 0};
const char COLOR_CONCEALED_BLUE_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '4', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_BLUE_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '4', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_RED_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '1', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_BLUE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '4', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_CYAN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '6', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_MAGENTA_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '5', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_CYAN_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '6', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_CYAN_RED_COLOREND[] = {0x1b, '[', '8', ';', '3', '6',
                                                  ';',  '4', '1', 'm', 0};
const char COLOR_CONCEALED_CYAN_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '6', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_CYAN_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '6', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_CYAN_BLUE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '6', ';', '4', '4', 'm', 0};
const char COLOR_CONCEALED_CYAN_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '6', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_CYAN_CYAN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '6', ';', '4', '6', 'm', 0};
const char COLOR_CONCEALED_CYAN_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '6', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_CYAN_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '6', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_WHITE_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_WHITE_RED_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '1', 'm', 0};
const char COLOR_CONCEALED_WHITE_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_WHITE_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_WHITE_BLUE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '4', 'm', 0};
const char COLOR_CONCEALED_WHITE_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_WHITE_CYAN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '6', 'm', 0};
const char COLOR_CONCEALED_WHITE_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_WHITE_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '7', ';', '4', '9', 'm', 0};
const char COLOR_CONCEALED_NORMAL_BLACK_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '0', 'm', 0};
const char COLOR_CONCEALED_NORMAL_RED_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '1', 'm', 0};
const char COLOR_CONCEALED_NORMAL_GREEN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '2', 'm', 0};
const char COLOR_CONCEALED_NORMAL_YELLOW_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '3', 'm', 0};
const char COLOR_CONCEALED_NORMAL_BLUE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '4', 'm', 0};
const char COLOR_CONCEALED_NORMAL_MAGENTA_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '5', 'm', 0};
const char COLOR_CONCEALED_NORMAL_CYAN_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '6', 'm', 0};
const char COLOR_CONCEALED_NORMAL_WHITE_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '7', 'm', 0};
const char COLOR_CONCEALED_NORMAL_NORMAL_COLOREND[] = {
    0x1b, '[', '8', ';', '3', '9', ';', '4', '9', 'm', 0};