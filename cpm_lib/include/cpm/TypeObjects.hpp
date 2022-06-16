#include "cpm/dds/ColorTypeObject.h"
#include "cpm/dds/CommonroadDDSGoalStateTypeObject.h"
#include "cpm/dds/CommonroadDDSShapeTypeObject.h"
#include "cpm/dds/CommonroadObstacleListTypeObject.h"
#include "cpm/dds/CommonroadObstacleTypeObject.h"
#include "cpm/dds/HeaderTypeObject.h"
#include "cpm/dds/HlcCommunicationTypeObject.h"
#include "cpm/dds/HLCHelloTypeObject.h"
#include "cpm/dds/LedPointsTypeObject.h"
#include "cpm/dds/LogLevelTypeObject.h"
#include "cpm/dds/LogTypeObject.h"
#include "cpm/dds/ParameterRequestTypeObject.h"
#include "cpm/dds/ParameterTypeObject.h"
#include "cpm/dds/Point2DTypeObject.h"
#include "cpm/dds/Pose2DTypeObject.h"
#include "cpm/dds/ReadyStatusTypeObject.h"
#include "cpm/dds/RoundTripTimeTypeObject.h"
#include "cpm/dds/StopRequestTypeObject.h"
#include "cpm/dds/SystemTriggerTypeObject.h"
#include "cpm/dds/TimeStampTypeObject.h"
#include "cpm/dds/VehicleCommandDirectTypeObject.h"
#include "cpm/dds/VehicleCommandTrajectoryTypeObject.h"
#include "cpm/dds/VehicleCommandPathTrackingTypeObject.h"
#include "cpm/dds/VehicleCommandSpeedCurvatureTypeObject.h"
#include "cpm/dds/VehicleObservationTypeObject.h"
#include "cpm/dds/VehicleStateListTypeObject.h"
#include "cpm/dds/VehicleStateTypeObject.h"
#include "cpm/dds/VisualizationTypeObject.h"

/**
 * Used as a workaround for issues with dynamic types in fastdds. (https://github.com/eProsima/Fast-DDS/issues/2184)
**/
void register_type_objects(){
    registerColorTypes();
    registerCommonroadDDSGoalStateTypes();
    registerCommonroadDDSShapeTypes();
    registerCommonroadObstacleListTypes();
    registerCommonroadObstacleTypes();
    registerHeaderTypes();
    registerHlcCommunicationTypes();
    registerHLCHelloTypes();
    registerLedPointsTypes();
    registerLogLevelTypes();
    registerLogTypes();
    registerParameterRequestTypes();
    registerParameterTypes();
    registerPoint2DTypes();
    registerPose2DTypes();
    registerReadyStatusTypes();
    registerRoundTripTimeTypes();
    registerStopRequestTypes();
    registerSystemTriggerTypes();
    registerTimeStampTypes();
    registerVehicleCommandDirectTypes();
    registerVehicleCommandPathTrackingTypes();
    registerVehicleCommandTrajectoryTypes();
    registerVehicleCommandSpeedCurvatureTypes();
    registerVehicleObservationTypes();
    registerVehicleStateListTypes();
    registerVehicleStateTypes();
    registerVisualizationTypes();
}