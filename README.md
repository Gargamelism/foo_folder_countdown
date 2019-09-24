# Foobar2000 Folder Countdown

## Description
This foobar2000 component adds the option to set a path and count, and it will notify you once the folder's content has been played the amount of times set.

## Usage
Copy the "foo_folder_countdown.dll" to C:\Program Files (x86)\foobar2000\components.
![configuration](https://i.ibb.co/XtVqr0K/foo-folder-countdown.png)

## Build
Download latest foobar2000 sdk from [foobar2000 sdk](https://www.foobar2000.org/SDK) and register the projects to the missing projects in the solution.
### Notes
* The SDK does not take kindly to std libs.
* Almost any change to the preferences requires creating a new GUID.
* If you're missing 'atlap.h', or any of the atl...h files you should add the projects include to [WTL](file:///E:/Program%20Files%20(x86)/WTL10_9163/ReadMe.html).
