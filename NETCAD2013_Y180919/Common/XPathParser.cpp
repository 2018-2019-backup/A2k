#include "StdAfx.h"
#include "XPathParser.h"
#include <sstream>

using namespace XPathNS;

XPathParser::XPathParser( std::string xmlFile ,bool fileType)
{
	using namespace MSXML2;

	CoInitialize(NULL);
	HRESULT hr;

	hr = pXMLDoc_.CreateInstance(__uuidof(MSXML2::DOMDocument60)) ;

	if (FAILED(hr))
	{
		//AfxMessageBox("hmmm.. Could not instantiate MSXML2::DOMDocument60 class");

		return;
	}
	if(fileType)
	{
		pXMLDoc_->async = VARIANT_FALSE;

		if(pXMLDoc_->load( xmlFile.c_str() ) != VARIANT_TRUE)
		{
			std::ostringstream errorStrm;
			errorStrm << "Did not load the xml file because: " << (LPCSTR)pXMLDoc_->parseError->Getreason();
			//AfxMessageBox( errorStrm.str().c_str() );

			return;
		}
	}
	else
		{
		if(pXMLDoc_->loadXML(xmlFile.c_str() ) != VARIANT_TRUE)
		{
			std::ostringstream errorStrm;
			errorStrm << "Did not load the xml file because: " << (LPCSTR)pXMLDoc_->parseError->Getreason();
			//AfxMessageBox( errorStrm.str().c_str() );

			return;
		}
	}

}

XPathParser::~XPathParser(void)
{
	pXMLDoc_.Release();

	CoUninitialize();
}


XMLNode XPathParser::selectSingleNode( std::string xPathExpression )
{
	XMLNode node;

	MSXML2::IXMLDOMNodePtr pNode = pXMLDoc_->selectSingleNode( xPathExpression.c_str() );

	node.name_ = (LPCSTR)pNode->nodeName;
	node.value_ = (LPCSTR)pNode->Gettext();
	node.xml_ = (LPCSTR)pNode->xml;

	// Do we have any attributes?
	MSXML2::IXMLDOMNamedNodeMapPtr attribs = pNode->Getattributes();
	if (attribs)
	{
		int cnt = attribs->Getlength();

		for ( int attribCnt  = 0; attribCnt < cnt; attribCnt++ )
		{
			MSXML2::IXMLDOMAttributePtr attrib = attribs->Getitem(attribCnt);

			XMLAttribute xmlAttrib;
			xmlAttrib.name_ = (LPCSTR)attrib->Getname();
			xmlAttrib.value_ = (LPCSTR)attrib->Gettext();

			node.nodeAttributes_.push_back( xmlAttrib );
		}
	}

	return node;
}

std::vector<XMLNode> &XPathParser::selectNodes( std::string xPathExpression )
{
	nodeList_.clear();

	MSXML2::IXMLDOMNodeListPtr pXMLNodeList = NULL;
	pXMLNodeList = pXMLDoc_->selectNodes( xPathExpression.c_str() );
	
	int count = pXMLNodeList->length;
	if(count==0)
	{
		//Commented for ACAD 2018
		//return  std::vector<XPathNS::XMLNode>();
		nodeList_ = std::vector<XPathNS::XMLNode>();
		return nodeList_;
	}
	MSXML2::IXMLDOMNodePtr pNode;
	for (int nodeCount = 0; nodeCount < pXMLNodeList->length; nodeCount++ )
	{
		XMLNode node;

		std::ostringstream strm;
		pNode = pXMLNodeList->item[nodeCount];
		
		node.name_ = (LPCSTR)pNode->nodeName;
		node.value_ = (LPCSTR)pNode->Gettext();
		node.xml_ = (LPCSTR)pXMLNodeList->item[nodeCount]->xml;

		// Do we have any attributes?
		MSXML2::IXMLDOMNamedNodeMapPtr attribs = pNode->Getattributes();
		if (attribs)
		{
			int cnt = attribs->Getlength();

			for ( int attribCnt  = 0; attribCnt < cnt; attribCnt++ )
			{
				MSXML2::IXMLDOMAttributePtr attrib = attribs->Getitem(attribCnt);

				XMLAttribute xmlAttrib;
				xmlAttrib.name_ = (LPCSTR)attrib->Getname();
				xmlAttrib.value_ = (LPCSTR)attrib->Gettext();

				node.nodeAttributes_.push_back( xmlAttrib );
			}
		}

		nodeList_.push_back(node);
	}

	pNode.Release();
	pXMLNodeList.Release();

	return nodeList_;
}
