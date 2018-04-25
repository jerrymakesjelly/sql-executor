#include <stdio.h>
#include <windows.h>
#include <sqlext.h>

#define BUFFER_SIZE 512

int ConnectToDatabase(SQLHDBC* const hdbc, SQLHENV* const henv, SQLCHAR* dsn, SQLCHAR* username, SQLCHAR* password)
{
    SQLRETURN retcode;

    // For environment handle
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, henv);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) // failed
    {
        puts("STOPPED: Unable to allocate environment handle.");
        return 0;
    }
    SQLSetEnvAttr(*henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    // For connection handle
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, *henv, hdbc);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) // failed
    {
        puts("STOPPED: Unable to allocate database controller handle.");
        return 0;
    }

    // Connect to database
    retcode = SQLConnect(*hdbc, (SQLCHAR*)dsn, SQL_NTS, username, SQL_NTS, password, SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) // failed
    {
        puts("STOPPED: Unable to connect.");
        return 0;
    }

    return 1;
}

void DisconnectToDatabase(SQLHDBC* const hdbc, SQLHENV* const henv)
{
    SQLDisconnect(*hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, *hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, *henv);
}

int PrintRecord(const SQLHDBC hdbc, SQLCHAR* sql)
{
    HSTMT hstmt;
    SQLRETURN retcode;
    SQLCHAR buffer[BUFFER_SIZE];
    SQLLEN length;
    unsigned int row = 0;
    SQLPOINTER* cols;
    SQLSMALLINT numColumn = 0, bufferLenUsed;
    const unsigned char magicnum = 20;
    char format[10];
    int i;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
    {
        puts("STOPPED: Unable to allocate statement handle.");
        return 0;
    }
    
    retcode = SQLExecDirect(hstmt, sql, SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
    {
        puts("STOPPED: Can't run this SQL statement.");
        return 0;
    }

    retcode = SQLNumResultCols(hstmt, &numColumn);
    if (retcode != SQL_SUCCESS)
    {
        puts("STOPPED: Unabled to get the number of columns.");
        return 0;
    }

    cols = (SQLPOINTER*)malloc(sizeof(SQLPOINTER) * numColumn);

    for (i = 0; i < numColumn; i++)
    {
        cols[i] = (SQLPOINTER)malloc(BUFFER_SIZE*sizeof(char));
        retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)i + 1, SQL_DESC_LABEL, cols[i], (SQLSMALLINT)BUFFER_SIZE, &bufferLenUsed, NULL);
        sprintf(format, "%%-%ds", magicnum);
        printf(format, cols[i]);
    }
    putchar('\n');

    if (retcode == SQL_SUCCESS)
    {
        while (TRUE)
        {
            retcode = SQLFetch(hstmt);
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
            {
                row++;
                for (i = 1; i <= numColumn; i++)
                {
                    SQLGetData(hstmt, i, SQL_C_CHAR, buffer, BUFFER_SIZE, &length);
                    sprintf(format, "%%-%ds", magicnum);
                    buffer[magicnum-1] = 0;
                    printf(format, length != -1 ? buffer : (SQLCHAR*)"");
                }
                putchar('\n');
            } else if (retcode == SQL_ERROR) {
                puts("STOPPED: An error occured while fetching records.");
                return 0;
            } else {
                break;
            }
        }
        printf("%d row(s).\n", row);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    return 1;
}

int ExecSQL(const SQLHDBC hdbc, SQLCHAR* sql)
{
    HSTMT hstmt;
    SQLRETURN retcode;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
    {
        puts("STOPPED: Unable to allocate statement handle.");
        return 0;
    }

    retcode = SQLExecDirect(hstmt, sql, SQL_NTS);
    
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    return retcode == SQL_SUCCESS;
}

int main(int argc, char* argv[])
{
    SQLHDBC hdbc;
    SQLHENV henv;

    SQLCHAR sql[BUFFER_SIZE];
    SQLCHAR temp;

    printf("SQL Executor @Zhaoyu Gan. BUILD TIME: %s %s\n\n", __DATE__, __TIME__);

    if(argc < 2)
    {
        printf("Usage: %s <data source name>\n", argv[0]);
        puts("No data source name specified.");
        exit(255);
    }

    // Connection Block
    if(!ConnectToDatabase(&hdbc, &henv, (SQLCHAR*)argv[1], 
        argc >= 3 ? (SQLCHAR*)argv[2] : NULL,
        argc >= 4 ? (SQLCHAR*)argv[3] : NULL))
    {
        puts("STOPPED: Can't connect to the data source. Please check your DSN, username and password, and try again.");
        exit(-1);
    }
    else
    {
        puts("Connect successfully.");
        puts("Note that if your SQL statement includes Chinese, you may need to set your console encoding to Simplified Chinese (GB2312).\n");
        puts("Type `quit` to quit.");
    }

    while(1)
    {
        puts("Input SQL:");
        printf("> ");
        fgets((char*)sql, sizeof(sql)/sizeof(sql[0]), stdin);

        if(!strcmp((char*)sql, "quit\n"))
            break;
        
        temp = sql[6];
        sql[6] = 0;
        if(!strcmp((char*)sql, "select")) // Select statement
        {
            sql[6] = temp;
            if(PrintRecord(hdbc, sql))
                puts("Execute successfully.");
            else
                puts("Failed. Please try again.");
        }
        else
        {
            sql[6] = temp;
            if(ExecSQL(hdbc, sql))
                puts("Execute successfully.");
            else
                puts("Failed. Please try again.");
        }
        putchar('\n');
    }

    puts("Goodbye!");

    return 0;
}