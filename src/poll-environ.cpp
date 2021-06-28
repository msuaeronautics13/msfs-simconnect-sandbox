#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "SimConnect.h"
#include <strsafe.h>

int     quit = 0;
HANDLE  hSimConnect = NULL;

struct Struct1
{
    char    title[256];
    double  velocity;
    double  clalpha;
    double  windspeed;
    double  winddirection;
};

static enum EVENT_ID {
    EVENT_RECUR_1SEC,
};

static enum DATA_DEFINE_ID {
    DEFINITION_1,
};

static enum DATA_REQUEST_ID {
    REQUEST_1,
};

void CALLBACK MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
    HRESULT hr;

    switch (pData->dwID)
    {
    case SIMCONNECT_RECV_ID_EVENT:
    {
        SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;

        switch (evt->uEventID)
        {
        case EVENT_RECUR_1SEC:

            // Request aircraft state every one second
            hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, REQUEST_1, DEFINITION_1, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);

            break;

        default:
            break;
        }
        break;
    }

    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
    {
        SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

        switch (pObjData->dwRequestID)
        {
        case REQUEST_1:
        {
            DWORD ObjectID = pObjData->dwObjectID;
            Struct1* pS = (Struct1*)&pObjData->dwData;
            if (SUCCEEDED(StringCbLengthA(&pS->title[0], sizeof(pS->title), NULL))) 
            {
                printf("\nObjectID=%d  aircraft=\"%s\"\nSpeed (kts)=%f  Direction (deg)=%f  CLalpha (1/rad)=%f  Total Vel (kts)=%.2f", ObjectID, pS->title, pS->windspeed, pS->winddirection, pS->clalpha, pS->velocity);
            }
            break;
        }

        default:
            break;
        }
        break;
    }


    case SIMCONNECT_RECV_ID_QUIT:
    {
        quit = 1;
        break;
    }

    default:
        printf("\nReceived:%d", pData->dwID);
        break;
    }
}

void testDataRequest()
{
    HRESULT hr;

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Request Data", NULL, 0, 0, 0)))
    {
        printf("\nConnected to MSFS!");

        // Set up the data definition, but do not yet do anything with it
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "title", NULL, SIMCONNECT_DATATYPE_STRING256);
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Total Velocity", "feet per second");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Linear CL Alpha", "per radian");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Ambient Wind Velocity", "knots");
        hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Ambient Wind Direction", "degrees");
       // hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Ambient Temperature", "celsius");
       // hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Beta Dot", "radians per second");
       // hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Plane Latitude", "degrees");
       // hr = SimConnect_AddToDataDefinition(hSimConnect, DEFINITION_1, "Plane Longitude", "degrees");

        hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_RECUR_1SEC, "1sec");

        while (0 == quit)
        {
            SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, NULL);
            Sleep(1);
        }

        hr = SimConnect_Close(hSimConnect);
    }
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{

    testDataRequest();

    return 0;
}
