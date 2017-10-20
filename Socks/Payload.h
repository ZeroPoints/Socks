#pragma once

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>




struct Payload
{

	enum PayloadType { None, Status, Info, Extra };

	PayloadType type;
	std::string data;

	template <class Archive>
	void serialize(Archive & ar)
	{
		ar(type, data);
	}

};



//Example
//
//std::stringstream ss;
//Payload payload;
//payload.data = "ace";
//
//payload.type = Payload::PayloadType::Extra;
//
//{
//	cereal::BinaryOutputArchive oarchive(ss);
//	oarchive(payload);
//}
//
//Payload payload2;
//{
//	cereal::BinaryInputArchive iarchive(ss);
//	iarchive(payload2);
//}



