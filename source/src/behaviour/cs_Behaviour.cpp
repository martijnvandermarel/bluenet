/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Dec 6, 2019
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <behaviour/cs_Behaviour.h>
#include <util/cs_WireFormat.h>
#include <drivers/cs_Serial.h>

Behaviour::Behaviour(
            Type typ,
            uint8_t intensity,
            uint8_t profileid,
            DayOfWeekBitMask activedaysofweek,
            TimeOfDay from, 
            TimeOfDay until) : 
        typ(typ),
        activeIntensity (intensity),
        profileId (profileid),
        activeDays(activedaysofweek),
        behaviourAppliesFrom (from),
        behaviourAppliesUntil (until)
    {
}

Behaviour::Behaviour(SerializedDataType arr) : 
    Behaviour(
        Type(WireFormat::deserialize<uint8_t>(arr.data() +             0,  WireFormat::size<uint8_t>())),
        WireFormat::deserialize<uint8_t>(arr.data() +             1,  WireFormat::size<uint8_t>()),
        WireFormat::deserialize<uint8_t>(arr.data() +             2,  WireFormat::size<uint8_t>()),
        WireFormat::deserialize<DayOfWeekBitMask>(arr.data() +    3,  WireFormat::size<DayOfWeekBitMask>()),
        WireFormat::deserialize<TimeOfDay>(arr.data() +           4,  WireFormat::size<TimeOfDay>()),
        WireFormat::deserialize<TimeOfDay>(arr.data() +           9,  WireFormat::size<TimeOfDay>())
    ){
    for(uint8_t b : arr){
        LOGd("basebehaviour constr: 0x%02x",b);
    }
}

Behaviour::SerializedDataType Behaviour::serialize() const{
    SerializedDataType result;
    auto result_iter = std::begin(result);

    // result_iter = std::copy_n( std::begin(getType()),        WireFormat::size<uint8_t>(),     result_iter);
    result_iter = std::copy_n( std::begin(WireFormat::serialize(activeIntensity)),        WireFormat::size<uint8_t>(),     result_iter);
    result_iter = std::copy_n( std::begin(WireFormat::serialize(profileId)),              WireFormat::size<uint8_t>(),     result_iter);
    result_iter = std::copy_n( std::begin(WireFormat::serialize(activeDays)),             WireFormat::size<uint8_t>(),     result_iter);
    result_iter = std::copy_n( std::begin(WireFormat::serialize(behaviourAppliesFrom)),   WireFormat::size<TimeOfDay>(),   result_iter);
    result_iter = std::copy_n( std::begin(WireFormat::serialize(behaviourAppliesUntil)),  WireFormat::size<TimeOfDay>(),   result_iter);

    // print();
    // print_ser(result);    

    return result;
}


uint8_t Behaviour::value() const {
    return activeIntensity;
}

TimeOfDay Behaviour::from() const {
    return behaviourAppliesFrom; 
}

TimeOfDay Behaviour::until() const {
    return behaviourAppliesUntil; 
}