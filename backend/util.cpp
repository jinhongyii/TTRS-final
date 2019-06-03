//
// Created by jinho on 5/10/2019.
//

#include "util.h"
namespace sjtu {

    DiskManager recorddisk("record.db");
    LogManager logManager(&recorddisk);
    BufferPoolManager recordbpm(RECORD_BUFFER_POOL_SIZE , &recorddisk , &logManager,&transactionManager);
    TransactionManager transactionManager(&logManager,&recordbpm);
LogRecovery logRecovery(&recorddisk,&recordbpm,&logManager,&transactionManager);

}