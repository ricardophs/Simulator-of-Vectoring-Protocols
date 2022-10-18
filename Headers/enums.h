#ifndef ENUMS_H
#define ENUMS_H

enum State {
    UPDATE,
    DIFFUSING,
};

enum SPECIAL_ATTR {
    NEUTRAL,
    INVALID,
};

enum EventType {
    ELECT,
    UPDATE_MSG,
    DIFFUSING_MSG,
    CLEAR_MSG,
    ADVERTISE,
    WITHDRAWAL,
    LINK_COST_CHANGE,
    LINK_FAILURE,
    LINK_ADDITION
};


#endif