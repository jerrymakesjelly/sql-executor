# SQL Executor
An SQL Executor in Database System Experiment 4.

![screenshot](https://user-images.githubusercontent.com/6760674/39241264-ded3ea4a-48b8-11e8-92ff-9cd1be5ca7aa.png)

## Compile
```bash
gcc ./sqlexecutor.c -o sqlexecutor.exe -lodbc32
```

## Usage
```bash
./sqlexecutor <data source name> [<username>] [<password>]
```

## Additional Notes
Please note that you must set up your data source by the following steps:

Run *odbcad32*, and add your User Data Source.

![odbcad32](https://user-images.githubusercontent.com/6760674/39241403-5814d306-48b9-11e8-8312-238efed61b02.png)