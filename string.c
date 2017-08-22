#include "B1404_LIB.h"
#include "Common.h"

#ifndef Puncture
_CONST char strVersion[] = "20170622 by version 1.0.0.7";
#else
_CONST char strVersion[] = "20170531 by version 1.0.0.4 for Puncture";
#endif

_CONST char strM0100[] = "*0100 RAP01PowerOn";
_CONST char strM0101[] = "*0101 PAR01Start";
_CONST char strM0102[] = "*0102 SlaveOK";
_CONST char strM0103[] = "*0103 SlaveConfigOK";
_CONST char strM0104[] = "*0104 RAP01StartOK";

//_CONST char strM0105[] = "*0105 Suspend";
//_CONST char strM0106[] = "*0106 Recover";

_CONST char strM0105[] = "*0105 Recover";
_CONST char strM0106[] = "*0106 Suspend";

_CONST char strM0110[] = "*0110 CurrentWork";
_CONST char strM0111[] = "*0111 StoreHumiture";
_CONST char strM0112[] = "*0112 CardStoreBusy";
_CONST char strM0199[] = "*0199 ReStart";
_CONST char strM0200[] = "*0200 DustbinState";
_CONST char strE0901[] = "!0901 SlaveConnectError";
_CONST char strE0902[] = "!0902 SlaveConfigError";
_CONST char strE0903[] = "!0903 RAP01StartFail";

_CONST char strE0910[] = "!0910 StoreOpenError";

_CONST char strM1101[] = "*1101 MachineRead";

_CONST char strE1901[] = "!1901 MotorError";
_CONST char strE1910[] = "!1910 TurnplateMotorLose";
_CONST char strE1911[] = "!1911 SamplingTurnMotorLose";
_CONST char strE1912[] = "!1912 SamplingNeedleMotorLose";
_CONST char strE1913[] = "!1913 CardLoadingMotorLose";
_CONST char strE1914[] = "!1914 CardUnLoadingMotorLose";
_CONST char strE1915[] = "!1915 CardLifterAMotorLose";
_CONST char strE1916[] = "!1916 CardLifterBMotorLose";
_CONST char strE1917[] = "!1917 CardStoreMoveMotorLose";
_CONST char strE1918[] = "!1918 DilutePumpMotorLose";
_CONST char strE1919[] = "!1919 LeanerPumpMotorLose";
_CONST char strE1920[] = "!1920 EffluentPumpMotorLose";
_CONST char strE1921[] = "!1921 SamplingSyringmotorLose";

_CONST char strE1930[] = "!1930 TurnplateMotorError";
_CONST char strE1931[] = "!1931 SamplingTurnMotorError";
_CONST char strE1932[] = "!1932 SamplingNeedleMotorError";
_CONST char strE1933[] = "!1933 CardLoadingMotorError";
_CONST char strE1934[] = "!1934 CardUnLoadingMotorError";
_CONST char strE1935[] = "!1935 CardLifterAMotorError";
_CONST char strE1936[] = "!1936 CardLifterBMotorError";
_CONST char strE1937[] = "!1937 CardStoreMoveMotorError";
_CONST char strE1938[] = "!1938 DilutePumpMotorError";
_CONST char strE1939[] = "!1939 LeanerPumpMotorError";
_CONST char strE1940[] = "!1940 EffluentPumpMotorError";
_CONST char strE1941[] = "!1941 SamplingSyringmotorError";

_CONST char strM2100[] = "*2100 StartLiquidCheck";
_CONST char strM2101[] = "*2101 PrimingLeanerDone";
_CONST char strM2102[] = "*2102 PrimingDiluentDone";
_CONST char strM2103[] = "*2103 CardUnloadInitDone";
_CONST char strM2104[] = "*2104 TurnPlateInitDone";
_CONST char strM2105[] = "*2105 LeanerSupply";
_CONST char strM2106[] = "*2106 DiluentSupply";
_CONST char strM2107[] = "*2107 TurnPlateZeroDone";
_CONST char strM2108[] = "*2108 CardStoreMoveInitDone";
_CONST char strM2109[] = "*2109 NeedleUpDownInitDone";

_CONST char strM2110[] = "*2110 DiluentPumpCalibValue";
_CONST char strM2111[] = "*2111 LeanerPumpCalibValue";
_CONST char strM2112[] = "*2112 EffluentPumpCalibValue";
_CONST char strM2113[] = "*2113 NeedleTurnInitDone";
_CONST char strM2114[] = "*2114 PlateCardUnloadDone";
_CONST char strM2115[] = "*2115 CardMoveHasCard";
_CONST char strM2116[] = "*2116 CardMoveCardUnloadDone";
_CONST char strM2117[] = "*2117 CardMoveAndPlateReady";
_CONST char strM2118[] = "*2118 SampPumpInitDone";

_CONST char strM2120[] = "*2120 LiquidCheckDone";

_CONST char strM2150[] = "*2150 CapDetectedCheck";

_CONST char strE2505[] = "!2505 LeanerLack";
_CONST char strE2506[] = "!2506 DiluentLack";

_CONST char strE2901[] = "!2901 PrimingLeanerError";
_CONST char strE2902[] = "!2902 PrimingDiluentError";

_CONST char strE2904[] = "!2904 WashLineError";
_CONST char strE2905[] = "!2905 NeedlerLineError";
_CONST char strE2906[] = "!2906 DiluentLineError";
//_CONST char strE2907[] = "!2907 LeanerLIneError";
_CONST char strE2908[] = "!2908 DrainLineError";

//_CONST char strE2910[] = "!2910 DiluentPumpCalibError";
//_CONST char strE2911[] = "!2911 LeanerPumpCalibError";
_CONST char strE2912[] = "!2912 EffluentPumpCalibError";

_CONST char strE2920[] = "!2920 LiquidCheckError";

_CONST char strE2950[] = "!2950 CapDetectedError";
_CONST char strE2951[] = "!2951 CapDetectedInvalid";
_CONST char strE2952[] = "!2952 DiluentSupplyEmpty";
_CONST char strE2953[] = "!2953 LeanerSupplyEmpty";
_CONST char strE2954[] = "!2954 DiluentAndLeanerSupplyEmpty";
_CONST char strE2955[] = "!2955 DiluentSupplyBubble";
_CONST char strE2956[] = "!2956 LeanerSupplyBubble";

_CONST char strM3100[] = "*3100 Startup";
_CONST char strM3101[] = "*3101 PleasePressStartKey";
_CONST char strM3102[] = "*3102 SampReady";
_CONST char strM3103[] = "*3103 SamplingStart";
_CONST char strM3104[] = "*3104 SamplingDone";
_CONST char strM3105[] = "*3105 GetCardDone";
_CONST char strM3106[] = "*3106 CardReady";
_CONST char strM3107[] = "*3107 Mixing1Start";
_CONST char strM3108[] = "*3108 Mixing2Start";
_CONST char strM3109[] = "*3109 MixReady";

_CONST char strM3110[] = "*3110 DeterminandReady";
_CONST char strM3111[] = "*3111 TestStart";
_CONST char strM3112[] = "*3112 ReadAStart";
_CONST char strM3113[] = "*3113 ReadBStart";
_CONST char strM3114[] = "*3114 ReadAClose";
_CONST char strM3115[] = "*3115 ReadBClose";
_CONST char strM3116[] = "*3116 CardUnload";
_CONST char strM3117[] = "*3117 CardUnloadAgain";
_CONST char strM3118[] = "*3118 WasteCardNonUseful";
_CONST char strM3119[] = "*3119 WithoutPunctureState";

_CONST char strM3120[] = "*3120 TestState";
_CONST char strM3121[] = "*3121 CurrentSamplingVolume";
_CONST char strM3122[] = "*3122 CardInserPlateDone";
_CONST char strM3123[] = "*3123 CurrentWaitPhotoFlag";
_CONST char strM3124[] = "*3124 CurrentCardScanfPos";
_CONST char strM3125[] = "*3125 SetGetCardTest";
_CONST char strM3126[] = "*3126 CardStoreGetCardRestart";
_CONST char strM3127[] = "*3127 CardStoreGetCardDone";
_CONST char strM3128[] = "*3128 CardStoreGetCardNum";
_CONST char strM3129[] = "*3129 CloseGetCardTest";

_CONST char strM3130[] = "*3130 CardStoreOpen";
_CONST char strM3131[] = "*3131 CardStoreClose";
_CONST char strM3132[] = "*3132 CardStoreFull";
_CONST char strM3133[] = "*3133 CardStoreLittle";
_CONST char strM3134[] = "*3134 CardStoreEmpty";
_CONST char strM3135[] = "*3135 PleaseCloseCardStore";
_CONST char strM3136[] = "*3136 CurrentDiluteRatio";
_CONST char strM3137[] = "*3137 CurrentCardStoreNum";
_CONST char strM3138[] = "*3138 CurrentReadTime1";
_CONST char strM3139[] = "*3139 CurrentReadTime2";

_CONST char strM3140[] = "*3140 CurrentReadModule";
_CONST char strM3141[] = "*3141 CurrentDropVolume";
_CONST char strM3142[] = "*3142 CurrentDropMode";
_CONST char strM3143[] = "*3143 CurrentReMixNum"; 
_CONST char strM3144[] = "*3144 ReMixNumNotUse";
_CONST char strM3145[] = "*3145 ReMixModeStart";
_CONST char strM3146[] = "*3146 TestBatchCheck";
_CONST char strM3147[] = "*3147 CurInsertRingNum";
_CONST char strM3148[] = "*3148 CurTestRingNum";
_CONST char strM3149[] = "*3149 GetCardState";

_CONST char strM3150[] = "*3150 AllStoreState";
_CONST char strM3151[] = "*3151 SpcStoreState";
_CONST char strM3152[] = "*3152 CurrentSleepTime";
_CONST char strM3153[] = "*3153 SamplingClose";
_CONST char strM3154[] = "*3154 SamplingOpen";
_CONST char strM3155[] = "*3155 SetReReadOK";
_CONST char strM3156[] = "*3156 CleanMode";
_CONST char strM3157[] = "*3157 StartCleaning";
_CONST char strM3158[] = "*3158 StartSuperCleaning";
_CONST char strM3159[] = "*3159 SetReadCloseOK";

_CONST char strM3160[] = "*3160 ManualPrimeDiluentStart";
_CONST char strM3161[] = "*3161 ManualPrimeFluidSart";
_CONST char strM3162[] = "*3162 ManualPrimeDiluentEnd";
_CONST char strM3163[] = "*3163 ManualPrimeFluidEnd";
//_CONST char strM3164[] = "*3164 DiluentCheckStart";
//_CONST char strM3165[] = "*3165 DiluentCheckEnd";
_CONST char strM3166[] = "*3166 WithoutMixture";
_CONST char strM3167[] = "*3167 CleaningDone";
_CONST char strM3168[] = "*3168 SuperCleaningDone";
_CONST char strM3169[] = "*3169 CleaningFailed";

_CONST char strM3170[] = "*3170 SuperCleaningFailed";
_CONST char strM3171[] = "*3171 ManualPrimeDiluentBusy";
_CONST char strM3172[] = "*3172 ManualPrimeFluidBusy";
_CONST char strM3173[] = "*3173 ManualFluidCheckBusy";
_CONST char strM3174[] = "*3174 CurrentGetVolumeForDrop";
_CONST char strM3175[] = "*3175 CurrentMonitorState";

_CONST char strM3188[] = "*3188 CurrentNeedleADC";

_CONST char strM3190[] = "*3190 Standby";
_CONST char strM3191[] = "*3191 SampingSleep";

_CONST char strM3194[] = "*3194 CardUnloadCountDone";
_CONST char strM3195[] = "*3195 CardLoadCountDone";

_CONST char strM3199[] = "*3199 ReMixTureOK";

_CONST char strM3200[] = "*3200 PressStartKeyDone";
_CONST char strM3201[] = "*3201 DiluteStepState";


_CONST char strM3217[] = "*3217 SampleCheck";
_CONST char strM3218[] = "*3218 DropVolumeFactor";

_CONST char strM3225[] = "*3225 BloodCollectionTubeOn";
_CONST char strM3226[] = "*3226 PunctureDistanceDone";
_CONST char strM3227[] = "*3227 LiquidLevelValue";


_CONST char strM3333[] = "*3333 CardNoneUseful";

_CONST char strE3902[] = "!3902 AutoLeanerPumpCalibError";

_CONST char strE3904[] = "!3904 AutoDiluentPumpCalibError";
_CONST char strE3905[] = "!3905 CleanModeLeanerEmpty";
_CONST char strE3906[] = "!3906 WasteCardStoreIsOpen";

_CONST char strE3911[] = "!3911 ManualLeanerPumpCalibError";
_CONST char strE3912[] = "!3912 ManualDiluentPumpCalibError";


_CONST char strE3925[] = "!3925 BloodCollectionTubeOff";
_CONST char strE3926[] = "!3926 PunctureDistanceUndo";
_CONST char strE3927[] = "!3927 NoLiquidLevelDetected";
_CONST char strE3928[] = "!3928 Cleaningheadpollution";

_CONST char strE3933[] = "!3933 CardUnloadCountError";
_CONST char strE3934[] = "!3934 CardLoadCountError";
_CONST char strE3935[] = "!3935 FindRingNumError";
_CONST char strE3936[] = "!3936 DropHeightFactorIsTrunZero";

_CONST char strM4101[] = "*4101 TurnPlateAdjustReady";
_CONST char strM4102[] = "*4102 NeedleOnMixSetDone";
_CONST char strM4104[] = "*4104 DropHeightSetDone";
_CONST char strM4105[] = "*4105 CardLoadStartAdjustReady";
_CONST char strM4106[] = "*4106 CardLoadEndAdjustDone";
_CONST char strM4107[] = "*4107 CardUnloadStartAdjustReady";
_CONST char strM4108[] = "*4108 CardUnloadEndAdjustDone";
_CONST char strM4109[] = "*4109 LiquidPhotoAdjustResult";
_CONST char strM4110[] = "*4110 LiquidPhotoAdjustDone";
_CONST char strM4111[] = "*4111 CardStorePhotoAdjustResult";
_CONST char strM4112[] = "*4112 CardStorePhotoAdjustDone";
_CONST char strM4113[] = "*4113 LampAdjustDone";
_CONST char strM4114[] = "*4114 LampLumValue";
_CONST char strM4115[] = "*4115 LampTurnOn";
_CONST char strM4116[] = "*4116 LampTurnOff";
_CONST char strM4117[] = "*4117 CalibResult";
_CONST char strM4118[] = "*4118 MixHeightSetDone";
_CONST char strM4119[] = "*4119 MixHeightCheckDone";
_CONST char strM4120[] = "*4120 TurnPlateCheckDone";
_CONST char strM4121[] = "*4121 NeedleTurnCheckDone";
_CONST char strM4122[] = "*4122 NeedleUpdownCheckDone";
_CONST char strM4123[] = "*4123 CardStoreMoveCheckDone";
_CONST char strM4124[] = "*4124 CardTakeHookCheckDone";
_CONST char strM4125[] = "*4125 CardLoadCheckDone";
_CONST char strM4126[] = "*4126 CardUnloadCheckDone";
_CONST char strM4127[] = "*4127 TestALifterCheckDone";
_CONST char strM4128[] = "*4128 TestBLifterCheckDone";
_CONST char strM4129[] = "*4129 DiluentPumpCheckDone";
_CONST char strM4130[] = "*4130 LeanerPumpCheckDone";
_CONST char strM4131[] = "*4131 EffluentPumpCheckDone";
_CONST char strM4132[] = "*4132 SamplingSryingCheckDone";
_CONST char strM4133[] = "*4133 LiquidPhotoSignal";
_CONST char strM4134[] = "*4134 LiquidPhotoCheckDone";
_CONST char strM4135[] = "*4135 CardStorePhotoSignal";
_CONST char strM4136[] = "*4136 CardStorePhotoCheckDone";
_CONST char strM4137[] = "*4137 NeeleOnMixSideCheckDone";
_CONST char strM4138[] = "*4138 DropHeightCheckDone";


_CONST char strM4150[] = "*4150 DiluentQuantifyTestReady";
_CONST char strM4151[] = "*4151 DiluentQuantifyTestQuit";
_CONST char strM4152[] = "*4152 LeanerQuantifyTestReady";
_CONST char strM4153[] = "*4153 LeanerQuantifyTestQuit";
_CONST char strM4154[] = "*4154 QuantifySuctionFinish";

_CONST char strM4174[] = "*4174 LiquidDetSignal";
_CONST char strM4175[] = "*4175 CardLoadDetSignal";
_CONST char strM4176[] = "*4176 CardUnloadDetSignal";

_CONST char strM4199[] = "*4199 CardGetCheckDone";

_CONST char strM4201[] = "*4201 SensorState";
_CONST char strM4202[] = "*4202 AllSensorState";

_CONST char strE4901[] = "!4901 TurnPlateAdjustError";
_CONST char strE4902[] = "!4902 NeedleOnMixSideAdjustError";
_CONST char strE4903[] = "!4903 NeedleOnMixCentreAdjustError";
_CONST char strE4904[] = "!4904 NeedleHeightAdjustError";
_CONST char strE4905[] = "!4905 CardLoadStartAdjustError";
_CONST char strE4906[] = "!4906 CardLoadEndAdjustError";
_CONST char strE4907[] = "!4907 CardUnloadStartAdjustError";
_CONST char strE4908[] = "!4908 CardUnloadEndAdjustError";
_CONST char strE4909[] = "!4909 CardUnloadSensorError";
_CONST char strE4910[] = "!4910 CardloadSensorError";

_CONST char strE4920[] = "!4920 TurnPlateCheckError";
_CONST char strE4921[] = "!4921 NeedleTurnCheckError";
_CONST char strE4922[] = "!4922 NeedleUpdownCheckError";
_CONST char strE4923[] = "!4923 CardStoreMoveCheckError";
_CONST char strE4924[] = "!4924 CardTakeCheckError";
_CONST char strE4925[] = "!4925 CardLoadCheckError";
_CONST char strE4926[] = "!4926 CardUnloadCheckError";
_CONST char strE4927[] = "!4927 TestALifterCheckError";
_CONST char strE4928[] = "!4928 TestBLifterCheckError";
_CONST char strE4929[] = "!4929 DiluentPumpCheckError";
_CONST char strE4930[] = "!4930 LeanerPumpCheckError";
_CONST char strE4931[] = "!4931 EffluentPumpCheckError";
_CONST char strE4932[] = "!4932 SamplingSryingCheckError";



