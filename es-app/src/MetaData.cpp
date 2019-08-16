#include "MetaData.h"

#include "utils/FileSystemUtil.h"
#include "Log.h"
#include <pugixml/src/pugixml.hpp>

MetaDataDecl gameDecls[] = {
	// key,         type,                   default,            statistic,  name in GuiMetaDataEd,  prompt in GuiMetaDataEd
	{"name",        MD_STRING,              "",                 false,      "이름",                 "게임 이름 입력"},
	{"sortname",    MD_STRING,              "",                 false,      "정렬용 이름",             "정렬에 사용할 이름 입력"},
	{"desc",        MD_MULTILINE_STRING,    "",                 false,      "설명",          "설명 입력"},
	{"image",       MD_PATH,                "",                 false,      "이미지",                "이미지 경로 입력"},
	{"video",       MD_PATH     ,           "",                 false,      "동영상",                "동영상 경로 입력"},
	{"marquee",     MD_PATH,                "",                 false,      "마키",              "마키 이미지 경로 입력"},
	{"thumbnail",   MD_PATH,                "",                 false,      "미리보기",            "미리보기 이미지 경로 입력"},
	{"rating",      MD_RATING,              "0.000000",         false,      "평점",               "평점 입력"},
	{"releasedate", MD_DATE,                "날짜없음",  false,      "발매일",         "발매일 입력"},
	{"developer",   MD_STRING,              "알수없음",          false,      "개발사",            "개발사 입력"},
	{"publisher",   MD_STRING,              "알수없음",          false,      "판매사",            "판매사 입력"},
	{"genre",       MD_STRING,              "알수없음",          false,      "장르",                "장르 입력"},
	{"players",     MD_INT,                 "1",                false,      "플레이어",              "플레이어 수 입력"},
	{"favorite",    MD_BOOL,                "false",            false,      "즐겨찾기",             "즐겨찾기 표시"},
	{"hidden",      MD_BOOL,                "false",            false,      "숨김",               "숨기기 설정/해제" },
	{"kidgame",     MD_BOOL,                "false",            false,      "아동용",              "아동용 게임 설정/해제" },
	{"playcount",   MD_INT,                 "0",                true,       "실행 횟수",           "실행 횟수 입력"},
	{"lastplayed",  MD_TIME,                "0",                true,       "최근 실행",          "최근 실행 일시 입력"}
};
const std::vector<MetaDataDecl> gameMDD(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0]));

MetaDataDecl folderDecls[] = {
	{"name",        MD_STRING,              "",                 false,      "이름",                 "게임 이름 입력"},
	{"sortname",    MD_STRING,              "",                 false,      "정렬용 이름",             "정렬에 사용할 이름 입력"},
	{"desc",        MD_MULTILINE_STRING,    "",                 false,      "설명",          "설명 입력"},
	{"image",       MD_PATH,                "",                 false,      "이미지",                "이미지 경로 입력"},
	{"thumbnail",   MD_PATH,                "",                 false,      "미리보기",            "미리보기 이미지 경로 입력"},
	{"video",       MD_PATH,                "",                 false,      "동영상",                "동영상 경로 입력"},
	{"marquee",     MD_PATH,                "",                 false,      "마키",              "마키 이미지 경로 입력"},
	{"rating",      MD_RATING,              "0.000000",         false,      "평점",               "평점 입력"},
	{"releasedate", MD_DATE,                "날짜없음",  false,      "발매일",         "발매일 입력"},
	{"developer",   MD_STRING,              "알수없음",          false,      "개발사",            "개발사 입력"},
	{"publisher",   MD_STRING,              "알수없음",          false,      "판매사",            "판매사 입력"},
	{"genre",       MD_STRING,              "알수없음",          false,      "장르",                "장르 입력"},
	{"players",     MD_INT,                 "1",                false,      "플레이어",              "플레이어 수 입력"}
};
const std::vector<MetaDataDecl> folderMDD(folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0]));

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type)
{
	switch(type)
	{
	case GAME_METADATA:
		return gameMDD;
	case FOLDER_METADATA:
		return folderMDD;
	}

	LOG(LogError) << "Invalid MDD type";
	return gameMDD;
}



MetaDataList::MetaDataList(MetaDataListType type)
	: mType(type), mWasChanged(false)
{
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
		set(iter->key, iter->defaultValue);
}


MetaDataList MetaDataList::createFromXML(MetaDataListType type, pugi::xml_node& node, const std::string& relativeTo)
{
	MetaDataList mdl(type);

	const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
	{
		pugi::xml_node md = node.child(iter->key.c_str());
		if(md)
		{
			// if it's a path, resolve relative paths
			std::string value = md.text().get();
			if (iter->type == MD_PATH)
			{
				value = Utils::FileSystem::resolveRelativePath(value, relativeTo, true);
			}
			mdl.set(iter->key, value);
		}else{
			mdl.set(iter->key, iter->defaultValue);
		}
	}

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node& parent, bool ignoreDefaults, const std::string& relativeTo) const
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for(auto mddIter = mdd.cbegin(); mddIter != mdd.cend(); mddIter++)
	{
		auto mapIter = mMap.find(mddIter->key);
		if(mapIter != mMap.cend())
		{
			// we have this value!
			// if it's just the default (and we ignore defaults), don't write it
			if(ignoreDefaults && mapIter->second == mddIter->defaultValue)
				continue;
			
			// try and make paths relative if we can
			std::string value = mapIter->second;
			if (mddIter->type == MD_PATH)
				value = Utils::FileSystem::createRelativePath(value, relativeTo, true);

			parent.append_child(mapIter->first.c_str()).text().set(value.c_str());
		}
	}
}

void MetaDataList::set(const std::string& key, const std::string& value)
{
	mMap[key] = value;
	mWasChanged = true;
}

const std::string& MetaDataList::get(const std::string& key) const
{
	return mMap.at(key);
}

int MetaDataList::getInt(const std::string& key) const
{
	return atoi(get(key).c_str());
}

float MetaDataList::getFloat(const std::string& key) const
{
	return (float)atof(get(key).c_str());
}

bool MetaDataList::isDefault()
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for (unsigned int i = 1; i < mMap.size(); i++) {
		if (mMap.at(mdd[i].key) != mdd[i].defaultValue) return false;
	}

	return true;
}

bool MetaDataList::wasChanged() const
{
	return mWasChanged;
}

void MetaDataList::resetChangedFlag()
{
	mWasChanged = false;
}
