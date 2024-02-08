#include <psp2/kernel/modulemgr.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h> 
#include <taihen.h>
#include <psp2/kernel/clib.h>

const int ANALOG_DEADZONE = 60;

SceTouchData touchData[64];
int writtenTouchData = 0;

int lStickEmuTimestamp = -1;
int rStickEmuTimestamp = -1;


static SceUID touchPeekHook;
const unsigned int NID_sceTouchPeek = 0xFF082DF0;
const unsigned int NID_sceTouchPeek2 = 0x3AD3D0A1;
const unsigned int NID_sceTouchRead = 0x169A1D58;
const unsigned int NID_sceTouchRead2 = 0x39401BEA;

double pow2(double a) {
    return a*a;
}

float xm1pw3p1(float f){
    return (f-1)*(f-1)*(f-1) + 1;
}

int abs(int x){
    return x < 0 ? -x : x;
}
int min(int a, int b){
    return a > b ? b : a;
}
int max(int a, int b){
    return a > b ? a : b;
}

void downShiftTouchDataBuffer(){
    for (int x = 1; x < 64; x++){
        touchData[x-1] = touchData[x];
    }
}

void pushTouchBuffer(SceTouchData data){
    if (++writtenTouchData == 64){
        downShiftTouchDataBuffer();
        writtenTouchData--;
    }
    touchData[writtenTouchData] = data;
}

static tai_hook_ref_t ref_hookTouchPeek;
static int touchPeekOverride(int port, SceTouchData* pData, int nBufs) {
    int rBuffers = TAI_CONTINUE(int, ref_hookTouchPeek, port, pData, nBufs);
    if (port == 1){     //port == 1 -> rear touch pad?
        SceTouchData* retTouchData = pData;
        SceCtrlData ctrlData[64];
        int readBuffers = sceCtrlPeekBufferPositive(0, ctrlData, nBufs);
        if (readBuffers != 0){

            SceCtrlData latestCtrlData = ctrlData[readBuffers-1];

            if (writtenTouchData == 0){
                SceTouchData newTouchData;
                newTouchData.reportNum = 0;
                newTouchData.status = 0;
                newTouchData.timeStamp = latestCtrlData.timeStamp;
                pushTouchBuffer(newTouchData);
            }
            SceTouchData editedTouchData = touchData[writtenTouchData];
            editedTouchData.timeStamp = ctrlData->timeStamp;
            editedTouchData.reportNum = 0;

            int stickDatas[] = {
                latestCtrlData.lx-127, latestCtrlData.ly-127, &lStickEmuTimestamp,
                latestCtrlData.rx-127, latestCtrlData.ry-127, &rStickEmuTimestamp
            };

            for (int x = 0; x < 2; x++){
                int xx = x * 3;
                if (abs(stickDatas[xx]) > ANALOG_DEADZONE || abs(stickDatas[xx+1]) > ANALOG_DEADZONE){
                    if (*(int*)stickDatas[xx+2] == -1){
                        *(int*)stickDatas[xx+2] = ctrlData->timeStamp;
                    }
                    int rStickEmuFrame = ctrlData->timeStamp - *(int*)stickDatas[xx+2];
                    SceTouchReport newReport;
                    newReport.id = x;
                    newReport.force = 255;
                    newReport.info = 0;
                    newReport.x = 100 + (x * 600);
                    newReport.y = 100 + (int)(xm1pw3p1(min(rStickEmuFrame, 500000) / 500000.0f) * 600);   //90 frames = 3 seconds
                    editedTouchData.report[editedTouchData.reportNum++] = newReport;
                    
                } else {
                    *(int*)stickDatas[xx+2] = -1;
                }
            }
            pushTouchBuffer(editedTouchData);

            //copy current touch buffer
            for (int i = 0; i < nBufs; i++){

                retTouchData[i] = touchData[max(writtenTouchData-nBufs + i, 0)];
            }
            return writtenTouchData+1;
            
        }
        return 0;
    }
    return rBuffers;
}



void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {

    //sceClibPrintf("divaf-touch-emu entry\n");

    touchPeekHook = taiHookFunctionImport(&ref_hookTouchPeek,
                                          TAI_MAIN_MODULE,
                                          TAI_ANY_LIBRARY,
                                          NID_sceTouchPeek,
                                          touchPeekOverride);
    
    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

    if (touchPeekHook != NULL) { 
        taiHookRelease(touchPeekHook, ref_hookTouchPeek); 
    }

    return SCE_KERNEL_STOP_SUCCESS;
}
