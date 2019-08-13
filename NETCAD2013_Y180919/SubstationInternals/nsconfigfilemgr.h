/*!
 @Class		ConfigFile
 @Brief		Provides functionality to read and write configuration file
 @Author	Neilsoft Ltd
 @Date		14 June 2007
 @History 	
 */

#pragma once
#pragma warning( disable: 4251 )
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "TCHAR.H"
#include "NSDirectives.h"

//using std::NSSTRING;

class __declspec(dllexport) ConfigFile {

public:
	// Methods
	ConfigFile(void);
	ConfigFile( TCHAR* szFilename,
	            NSSTRING szDelimiter = _T("="),
	            NSSTRING szComment = _T("#"),
				NSSTRING strSentry = _T("EndConfigFile" ));
	~ConfigFile(void);

	// Search for Key and read Value or optional default Value
	template<class T> T readValue( const NSSTRING& szKey ) const;  // call as readValue<T>
	template<class T> T readValue( const NSSTRING& szKey, const T& szValue ) const;
	template<class T> bool readInto( T& var, const NSSTRING& szKey ) const;
	bool readString( TCHAR* szValue,  TCHAR* szKey ) ;
	template<class T>	bool readInto( T& var, const NSSTRING& szKey, const T& szValue ) const;
	
	// Modify keys and values
	template<class T> void add( NSSTRING szKey, const T& szValue );
	void remove( const NSSTRING& szKey );
	void add( NSSTRING strKey, NSSTRING strValue);
	
	void SetPath( TCHAR* strFilename )	;

	// Check whether Key exists in configuration
	bool keyExists( const NSSTRING& szKey ) const;
	
	// Check or change configuration syntax
	NSSTRING getDelimiter() const { return m_szDelimiter; }
	NSSTRING getComment() const { return m_szComment; }
	NSSTRING getSentry() const { return m_szSentry; }
	NSSTRING setDelimiter( const NSSTRING& szDelimiter )
		{ NSSTRING szOld = m_szDelimiter;  m_szDelimiter = szDelimiter;  return szOld; }  
	NSSTRING setComment( const NSSTRING& szComment )
		{ NSSTRING szOld = m_szComment;  m_szComment = szComment;  return szOld; }
	
	NSISTREAM& AddToStream( NSISTREAM& is);
	NSOSTREAM& GetFromStream( NSOSTREAM& os);

	// Exception types
	struct FileNotFound {
		NSSTRING szFilename;
		FileNotFound( const NSSTRING& filename_ = NSSTRING() )
			: szFilename(filename_) {} };
	struct KeyNotFound {  // thrown only by T readValue(Key) variant of readValue()
		NSSTRING szKey;
		KeyNotFound( const NSSTRING& key_ = NSSTRING() )
			: szKey(key_) {} };

    void updateURLMap(std::map<NSSTRING,NSSTRING> & m_mapofURL, TCHAR* szSearchString);
	
protected:
	// members
	NSSTRING m_szDelimiter;  // separator between Key and Value
	NSSTRING m_szComment;    // separator between Value and comments
	NSSTRING m_szSentry;     // optional NSSTRING to signal end of file
	std::map<NSSTRING,NSSTRING> m_mapContents;  // extracted keys and values
	
	typedef std::map<NSSTRING,NSSTRING>::iterator m_MapIterator;
	typedef std::map<NSSTRING,NSSTRING>::const_iterator m_MapConstIterator;

	// methods
	template<class T> static NSSTRING T_as_string( const T& t );
	template<class T> static T string_as_T( const NSSTRING& s );
	static void trim( NSSTRING& s );
};