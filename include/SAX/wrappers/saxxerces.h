#ifndef ARABICA_SAX_XERCES_H
#define ARABICA_SAX_XERCES_H
//---------------------------------------------------------------------------
// A SAX2 wrapper class for Xerces.
//
// $Id$
//---------------------------------------------------------------------------

// Debugging code for the doSetProperty and doGetProperty methods.
// Since these methods use runtime casting, it is often helpful to
// know the exact type of a method's arguments to compare with
// what is expected.  This is hard, even with current (2003)
// debuggers, so this code can print it out, assuming you've
// compiled with gcc 3.2
//
// Example usage is given in doSetProperty

// Use only with GCC 3.2
#ifdef SAXXERCES_DEBUG

#include <iostream>
#include <typeinfo>
#include <cxxabi.h>

// Demangle Run-Time Type Information std::type_info struct.
std::ostream& operator<<(std::ostream& o, const std::type_info& ti)
{
  int status;
  char *realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
  if (status != 0)
    o.setstate(std::ios_base::failbit);
  o << realname;
  free(realname);
  return o;
}
#endif

#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include <SAX/Attributes.h>
#include <SAX/InputSource.h>
#include <SAX/XMLReader.h>
#include <SAX/ext/ProgressiveParser.h>
#include <SAX/SAXParseException.h>
#include <SAX/SAXNotRecognizedException.h>
#include <SAX/SAXNotSupportedException.h>
#include <SAX/ext/LexicalHandler.h>
#include <SAX/ext/DeclHandler.h>
#include <SAX/IStreamHandle.h>
#include <SAX/wrappers/XercesPropertyNames.h>
#include <SAX/wrappers/XercesFeatureNames.h>
#include <Utils/getparam.hpp>

// Xerces Includes
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/ContentHandler.hpp>
#include <xercesc/sax2/LexicalHandler.hpp>
#include <xercesc/sax2/DeclHandler.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/DTDHandler.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/sax/InputSource.hpp>

#include <xercesc/util/TransService.hpp>

#ifndef XERCES_HAS_CPP_NAMESPACE
#define XERCES_CPP_NAMESPACE
#endif

namespace SAX
{

namespace XercesImpl
{
  ///////////////////////////////
  // Xerces platform initializer
  /**
    * Class to handle calls to XMLPlatformUtils::Initialize() and
    * XMLPlatformUtils::Terminate().
    *
    * Without such a class, calls to Initialize() and Terminate() may be nested
    * and cause evil consequences.  Eg. A typical use of the old xerces_wrapper
    * could have resulted in:
    * <pre>
    * // Construct two xerces_wrappers.
    *     XMLPlatformUtils::Initialize()
    *     XMLPlatformUtils::Initialize()
    *     
    * // do stuff
    *
    * // Get rid of one of the xerces_wrappers
    *     XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate()
    *     
    * // do more stuff -- this is _after_ a call to Terminate()...
    *
    *     XMLPlatformUtils::Terminate()
    * </pre>
    */
  class xerces_initializer
  {
    public:
      xerces_initializer() { doInitialize(); }
      ~xerces_initializer() { doTerminate(); }

    private:
      void doInitialize();
      void doTerminate();

      static int count_;
  }; // class xerces_initializer
} // namespace XercesImpl

template<class string_type, 
         class T0 = Arabica::nil_t,
         class T1 = Arabica::nil_t>
class xerces_wrapper : public SAX::basic_ProgressiveParser<string_type>
{
  private:
    typedef typename Arabica::get_param<Arabica::string_adaptor_tag, 
                               Arabica::default_string_adaptor<string_type>, 
                               T0, 
                               T1>::type string_adaptor_type;
    typedef string_adaptor_type string_adaptorT;

  protected:
    typedef SAX::basic_XMLReader<string_type> base;

  public:
    typedef typename base::EntityResolverT EntityResolverT;
    typedef typename base::DTDHandlerT DTDHandlerT;
    typedef typename base::ContentHandlerT ContentHandlerT;
    typedef typename base::InputSourceT InputSourceT;
    typedef SAX::basic_LexicalHandler<string_type> LexicalHandlerT;
    typedef SAX::basic_Locator<string_type> LocatorT;
    typedef SAX::basic_Attributes<string_type> AttributesT;
    typedef SAX::basic_DeclHandler<string_type> DeclHandlerT;
    typedef SAX::basic_ErrorHandler<string_type> ErrorHandlerT;

    typedef SAX::XercesFeatureNames<string_type, string_adaptorT> featuresT;

    xerces_wrapper();
    virtual ~xerces_wrapper();

    /////////////////////////////////////////////////
    // Configuration
    virtual bool getFeature(const string_type& name) const;
    virtual void setFeature(const string_type& name, bool value);
  
    /////////////////////////////////////////////////
    // Event Handlers
    virtual void setEntityResolver(EntityResolverT& resolver) { entityResolverAdaptor_.setEntityResolver(resolver); }
    virtual EntityResolverT* getEntityResolver() const { return entityResolverAdaptor_.getEntityResolver(); }
    virtual void setDTDHandler(DTDHandlerT& handler) { dtdHandlerAdaptor_.setDTDHandler(handler); }
    virtual DTDHandlerT* getDTDHandler() const { return dtdHandlerAdaptor_.getDTDHandler(); }
    virtual void setContentHandler(ContentHandlerT& handler) { contentHandlerAdaptor_.setContentHandler(handler); }
    virtual ContentHandlerT* getContentHandler() const { return contentHandlerAdaptor_.getContentHandler(); }
    virtual void setErrorHandler(ErrorHandlerT& handler) { errorHandlerAdaptor_.setErrorHandler(handler); }
    virtual ErrorHandlerT* getErrorHandler() const { return errorHandlerAdaptor_.getErrorHandler(); }

    //////////////////////////////////////////////////
    // Parsing
    virtual void parse(InputSourceT& input);

    //////////////////////////////////////////////////
    // Progressive Parsing
    //////////////////////////////////////////////////

    /** @name Progressive Parsing */
    //@{
    virtual bool parseFirst(const string_type& systemId, XMLPScanToken& toFill)
    {
      InputSourceT is(systemId);
      return parseFirst(is, toFill);
    }

    virtual bool parseFirst(InputSourceT& input,
                            XMLPScanToken& toFill);

    virtual bool parseNext(XMLPScanToken& token);

    virtual void parseReset(XMLPScanToken& token);
    //@}

  protected:
    virtual std::auto_ptr<typename base::PropertyBase> doGetProperty(const string_type& name);
    virtual void doSetProperty(const string_type& name, std::auto_ptr<typename base::PropertyBase> value);

  private:
    ///////////////////////////////
    // String adaptor for XMLCh
    // * For the null-terminated version, we convert the array of XMLCh to a
    //   null-terminated vector of wchar_t.  We then pass &vector[0] to the
    //   parent method.
    // * For the known length version, we convert the the array of XMLCh to a
    //   vector of wchar_t.  We then pass &vector[0] and length to the parent
    //   method.
    template<typename char_type>
    class xerces_string_janitor
    {
      // Ordinarily you'd expect to be able to use Xerces' Array janitor class.  However,
      // Windows exes and dlls use seperate heaps, so you can't delete in one module that
      // was allocated by another.  It's a pita.  The solution here is to use XMLString::release.
      // Although this simply does a delete[], the deletion occurs in the Xerces DLL, so
      // it will work correctly.
    public:
      xerces_string_janitor(char_type* ptr) : ptr_(ptr) { }
      ~xerces_string_janitor() { XERCES_CPP_NAMESPACE::XMLString::release(&ptr_); }

      char_type const* const get() const { return ptr_; }

    private:
      char_type* ptr_;
      xerces_string_janitor(const xerces_string_janitor&);
      xerces_string_janitor& operator=(const  xerces_string_janitor&) const;
    }; // class xerces_string_janitor

    class xerces_string_adaptor : public string_adaptorT
    {
      public:
        typedef typename string_adaptorT::value_type value_type;
        typedef string_adaptorT base;

        typedef std::vector<wchar_t> wVector;

#ifndef ARABICA_NO_WCHAR_T
        static string_type makeStringT(const XMLCh* str) 
        {
          if(str)
          {
            wVector buffer;
            std::insert_iterator<wVector> inserter(buffer, buffer.begin());

            // We want the trailing 0 character.
            *inserter = *str;
            while (*str != 0)  // str points to the character we've just copied
            {
              *++inserter = *++str;
            }
            return base::construct_from_utf16(&buffer[0]);
          }
          else
          return string_type();
        } // makeStringT

        static string_type makeStringT(const XMLCh* str, int length) 
        {
          if (str && length)
          {
            wVector buffer;
            std::insert_iterator<wVector> inserter(buffer, buffer.begin());

            // We don't want any trailing characters.
            const XMLCh* end = str + length;
            while (str < end)
            {
              *inserter++ = *str++;
            }
            return base::construct_from_utf16(&buffer[0], length);
          }
          else
            return string_type();
        } // makeStringT
#else
        // alternative version for the wchar_t impaired
        static string_type makeStringT(const XMLCh* str) 
        {
          if(str)
          {
            xerces_string_janitor<char> cstr(XERCES_CPP_NAMESPACE::XMLString::transcode(str));
            return base::construct_from_utf8(cstr.get());
          }
          return string_type();
        } // makeStringT

        static string_type makeStringT(const XMLCh* str, int length) 
        {
          // this isn't pretty, but Xerces doesn't provide a transcode with takes
          // a length
          if(str && length)
          {
            std::vector<XMLCh> wv(length + 1);
            std::copy(str, str+length, std::insert_iterator<std::vector<XMLCh> >(wv, wv.begin()));
            wv.push_back(0);

            return makeStringT(&wv[0]);
          }
          return string_type();
        } // makeStringT
#endif
        static XMLCh* asXMLChString(const string_type& s) 
        {
          std::string str = base::asStdString(s);
          return XERCES_CPP_NAMESPACE::XMLString::transcode(str.c_str());
        } // asXMLChString
    }; // class xerces_string_adaptor

    typedef xerces_string_adaptor SA;

    ///////////////////////////////
    // Adaptors
    class LocatorAdaptor : public LocatorT
    {
      public:
        LocatorAdaptor() : locator_(0) { }
        LocatorAdaptor(const XERCES_CPP_NAMESPACE::Locator* const locator) : locator_(locator) { } 
        ~LocatorAdaptor() { }

        string_type getPublicId() const
        {
          if(!locator_) 
            return string_type();

          return xerces_string_adaptor::makeStringT(locator_->getPublicId());
        } // getPublicId
    
        string_type getSystemId() const
        {
          if(!locator_)
            return string_type();

          return SA::makeStringT(locator_->getSystemId());
        } // getSystemId
    
        int getLineNumber() const
        {
          if(!locator_)
            return -1;

          return locator_->getLineNumber();
        } // getLineNumber
    
        int getColumnNumber() const
        {
          if(!locator_)
            return -1;

          return locator_->getColumnNumber();
        } // getColumnNumber
    
        void setLocator(const XERCES_CPP_NAMESPACE::Locator* const locator)
        {
          locator_ = locator;
        } // setLocator

      private:
        const XERCES_CPP_NAMESPACE::Locator* locator_;
    }; // class LocatorAdaptor

    class EntityResolverAdaptor : public XERCES_CPP_NAMESPACE::EntityResolver
    {
      public:
        EntityResolverAdaptor() : entityResolverHandler_(0) { }
        ~EntityResolverAdaptor() { }

        void setEntityResolver(EntityResolverT& handler) 
              {
                entityResolverHandler_ = &handler; 
              }
        EntityResolverT* getEntityResolver() const
        {
            return entityResolverHandler_;
        }

        virtual XERCES_CPP_NAMESPACE::InputSource* resolveEntity(const XMLCh* const publicId,
                                           const XMLCh* const systemId)
              {
                InputSourceT source;
            if(entityResolverHandler_)
                  {
                    source = entityResolverHandler_->resolveEntity(SA::makeStringT(publicId),
                                                           SA::makeStringT(systemId));
                  }
                  // If all of these are empty, the EntityResolver has requested that we
                  // use the default entity resolver.
                  if ((source.getByteStream() == 0) &&
                      (SA::empty(source.getPublicId())) &&
                      (SA::empty(source.getSystemId())))
                  {
                // Xerces-C takes a NULL (0) return to mean "use the default entity
                // resolver".
                      return 0;
                  }
                  else
                  {
                      return new InputSourceAdaptor(source);
                  }
              }

      private:
        EntityResolverT* entityResolverHandler_;
    }; // class EntityResolverAdaptor

    class DTDHandlerAdaptor : public XERCES_CPP_NAMESPACE::DTDHandler 
    {
      public:
        DTDHandlerAdaptor() : dtdHandler_(0) { }
        ~DTDHandlerAdaptor() { }

        void setDTDHandler(DTDHandlerT& handler) { dtdHandler_ = &handler; }
        DTDHandlerT* getDTDHandler() const { return dtdHandler_; }

        virtual void notationDecl(const XMLCh* const name,
                                  const XMLCh* const publicId, 
                                  const XMLCh* const systemId)
        {
          if(dtdHandler_)
            dtdHandler_->notationDecl(SA::makeStringT(name),
                                      SA::makeStringT(publicId),
                                      SA::makeStringT(systemId));
        } // notationDecl
        virtual void unparsedEntityDecl(const XMLCh* const name,
                                        const XMLCh* const publicId, 
                                        const XMLCh* const systemId,
                                        const XMLCh* const notationName)
        {
          if(dtdHandler_)
            dtdHandler_->unparsedEntityDecl(SA::makeStringT(name),
                                            SA::makeStringT(publicId),
                                            SA::makeStringT(systemId),
                                            SA::makeStringT(notationName));
        } // unparsedEntityDecl

        virtual void resetDocType() 
        {
          // no op
        } // resetDocType

      private:
        DTDHandlerT* dtdHandler_;
    }; // class DTDHandlerAdaptor

    class ContentHandlerAdaptor : public XERCES_CPP_NAMESPACE::ContentHandler 
    {
      public:
        ContentHandlerAdaptor() : contentHandler_(0) { } 
        ~ContentHandlerAdaptor() { }

        void setContentHandler(ContentHandlerT& handler) { contentHandler_ = &handler; }
        ContentHandlerT* getContentHandler() const { return contentHandler_; }

        virtual void setDocumentLocator(const XERCES_CPP_NAMESPACE::Locator* const locator)
        {
          locator_.setLocator(locator);
          if(contentHandler_) 
            contentHandler_->setDocumentLocator(locator_);
        } // putDocumentLocation
        
        virtual void startDocument()
        {
          if(contentHandler_) 
            contentHandler_->startDocument();
        } // startDocument
        
        virtual void endDocument()
        {
          if(contentHandler_) 
            contentHandler_->endDocument();
        } // endDocument
        
        virtual void startPrefixMapping(const XMLCh* const prefix, const XMLCh* const uri)
        {
          if(contentHandler_)
            contentHandler_->startPrefixMapping(SA::makeStringT(prefix), SA::makeStringT(uri));
        } // startPrefixMapping
       
        virtual void endPrefixMapping(const XMLCh* const prefix)
        {
          if(contentHandler_)
            contentHandler_->endPrefixMapping(SA::makeStringT(prefix));
        } // endPrefixMapping
        
        virtual void startElement(const XMLCh* const namespaceUri,
                                  const XMLCh* const localName,
                                  const XMLCh* const rawName,
                                  const XERCES_CPP_NAMESPACE::Attributes& attrs)
        {
          if(contentHandler_)
          {
            AttributesAdaptor attrAdaptor(attrs);
            contentHandler_->startElement(SA::makeStringT(namespaceUri),
                                          SA::makeStringT(localName),
                                          SA::makeStringT(rawName),
                                          attrAdaptor);
          } // if ...
        } // startElement
        
        virtual void endElement(const XMLCh* const namespaceUri,
                                const XMLCh* const localName,
                                const XMLCh* const rawName)
        {
          if(contentHandler_)
            contentHandler_->endElement(SA::makeStringT(namespaceUri),
                                        SA::makeStringT(localName),
                                        SA::makeStringT(rawName));
        } // endElement
        
        virtual void characters(const XMLCh* const chars, const unsigned int length)
        {
          if(contentHandler_) 
            contentHandler_->characters(SA::makeStringT(chars, length));
        } // characters
        
        virtual void ignorableWhitespace(const XMLCh* const chars, const unsigned int length)
        {
          if(contentHandler_)
            contentHandler_->ignorableWhitespace(SA::makeStringT(chars, length));
        } // ignorableWhitespace
        
        virtual void processingInstruction(const XMLCh* const target,
                                           const XMLCh* const data)
        {
          if(contentHandler_)
            contentHandler_->processingInstruction(SA::makeStringT(target),
                                                   SA::makeStringT(data));
        } // processingInstruction
        
        virtual void skippedEntity(const XMLCh* const name)
        {
          if(contentHandler_)
            contentHandler_->skippedEntity(SA::makeStringT(name));
        } // skippedEntity

      private:
        ////////////////////////////////////////////////
        // member varaibles
        ContentHandlerT* contentHandler_;
        LocatorAdaptor locator_;

        //////////////////////////////////////////////////////
        // COM interface -> C++ interface adaptors
        class AttributesAdaptor : public AttributesT
        {
          public:
            AttributesAdaptor(const XERCES_CPP_NAMESPACE::Attributes& attrs) : attributes_(attrs) { }
            ~AttributesAdaptor() { }

            /////////////////////////
            // indexed access
            virtual int getLength() const
            {
              return attributes_.getLength();
            } // getLength
            virtual string_type getURI(unsigned int index) const
            {
              return SA::makeStringT(attributes_.getURI(index));
            } // getURI
            virtual string_type getLocalName(unsigned int index) const
            {
              return SA::makeStringT(attributes_.getLocalName(index));
            } // getLocalName
            virtual string_type getQName(unsigned int index) const
            {
              return SA::makeStringT(attributes_.getQName(index));
            } // getQName
            virtual string_type getType(unsigned int index) const
            {
              return SA::makeStringT(attributes_.getType(index));
            } // getType
            virtual string_type getValue(unsigned int index) const
            {
              return SA::makeStringT(attributes_.getValue(index));
            } // getValue

            /////////////////////////
            // name based query
            virtual int getIndex(const string_type& uri, const string_type& localName) const
            {
              xerces_string_janitor<XMLCh> wUri(SA::asXMLChString(uri));
              xerces_string_janitor<XMLCh> wLocalName(SA::asXMLChString(localName));
              return attributes_.getIndex(wUri.get(), wLocalName.get());
            } // getIndex
            virtual int getIndex(const string_type& qName) const
            {
              xerces_string_janitor<XMLCh> wQName(SA::asXMLChString(qName));
              return attributes_.getIndex(wQName.get());
            } // getIndex
            virtual string_type getType(const string_type& uri, const string_type& localName) const
            {
              xerces_string_janitor<XMLCh> wUri(SA::asXMLChString(uri));
              xerces_string_janitor<XMLCh> wLocalName(SA::asXMLChString(localName));
              return SA::makeStringT(attributes_.getType(wUri.get(), wLocalName.get()));
            } // getType
            virtual string_type getType(const string_type& qName) const
            {
              xerces_string_janitor<XMLCh> wQName(SA::asXMLChString(qName));
              return SA::makeStringT(attributes_.getType(wQName.get()));
            } // getType
            virtual string_type getValue(const string_type& uri, const string_type& localName) const
            {
              xerces_string_janitor<XMLCh> wUri(SA::asXMLChString(uri));
              xerces_string_janitor<XMLCh> wLocalName(SA::asXMLChString(localName));
              return SA::makeStringT(attributes_.getValue(wUri.get(), wLocalName.get()));
            } // getValue
            virtual string_type getValue(const string_type& qname) const
            {
              xerces_string_janitor<XMLCh> wQName(SA::asXMLChString(qname));
              return SA::makeStringT(attributes_.getValue(wQName.get()));
            } // getValue

          private:
            const XERCES_CPP_NAMESPACE::Attributes& attributes_;

            AttributesAdaptor();
        }; // class AttributesAdaptor
    }; // class ContentHandlerAdaptor

    class ErrorHandlerAdaptor : public XERCES_CPP_NAMESPACE::ErrorHandler 
    {
      public:
        ErrorHandlerAdaptor() :   errorHandler_(0) { }
        virtual ~ErrorHandlerAdaptor() { }

        void setErrorHandler(ErrorHandlerT& handler) { errorHandler_ = &handler; }
        ErrorHandlerT* getErrorHandler() const { return errorHandler_; }

        virtual void warning(const XERCES_CPP_NAMESPACE::SAXParseException& exception)
        {
          handleError(exception, &ErrorHandlerT::warning);
        } // warning

        virtual void error(const XERCES_CPP_NAMESPACE::SAXParseException& exception)
        {
          handleError(exception, &ErrorHandlerT::error);
        } // error

        virtual void fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& exception)
        {
          handleError(exception, &ErrorHandlerT::fatalError);
        } // fatalError

        virtual void resetErrors() 
        { 
          // no op 
        } // resetErrors

      private:
        typedef void(ErrorHandlerT::* ErrorFn)(const typename ErrorHandlerT::SAXParseExceptionT&);

        void handleError(const XERCES_CPP_NAMESPACE::SAXParseException& exception, ErrorFn fn)
        {
          if(!errorHandler_)
            return;

          string_type errorMsg(SA::makeStringT(exception.getMessage()));
          string_type publicId(SA::makeStringT(exception.getPublicId()));
          string_type systemId(SA::makeStringT(exception.getSystemId()));
          ErrorHandlerT::SAXParseExceptionT sp(SA::asStdString(errorMsg),
                                               publicId,
                                               systemId,
                                               exception.getLineNumber(),
                                               exception.getColumnNumber());
          (errorHandler_->*fn)(sp);
        } // handleError

        ErrorHandlerT* errorHandler_;
    }; // class ErrorHandlerAdaptor

    class LexicalHandlerAdaptor : public XERCES_CPP_NAMESPACE::LexicalHandler 
    {
      public:
        LexicalHandlerAdaptor() : lexicalHandler_(0) { }
        virtual ~LexicalHandlerAdaptor() { }

        void setLexicalHandler(LexicalHandlerT& handler) { lexicalHandler_ = &handler; }
        LexicalHandlerT* getLexicalHandler() const { return lexicalHandler_; }

        virtual void startDTD(const XMLCh* const name,
                              const XMLCh* const publicId,
                              const XMLCh* const systemId)
        {
          if(lexicalHandler_)
            lexicalHandler_->startDTD(SA::makeStringT(name),
                                      SA::makeStringT(publicId),
                                      SA::makeStringT(systemId));
        } // startDTD

        virtual void endDTD()
        {
          if(lexicalHandler_)
            lexicalHandler_->endDTD();
        } // endDTD

        virtual void startEntity(const XMLCh* const name)
        {
          if(lexicalHandler_)
            lexicalHandler_->startEntity(SA::makeStringT(name));
        } // startEntity

        virtual void endEntity(const XMLCh* const name)
        {
          if(lexicalHandler_)
            lexicalHandler_->endEntity(SA::makeStringT(name));
        } // endEntity

        virtual void startCDATA()
        {
          if(lexicalHandler_)
            lexicalHandler_->startCDATA();
        } // startCDATA

        virtual void endCDATA()
        {
          if(lexicalHandler_)
            lexicalHandler_->endCDATA();
        } // endCDATA

        virtual void comment(const XMLCh* const chars, const unsigned int length)
        {
          if(lexicalHandler_)
            lexicalHandler_->comment(SA::makeStringT(chars, length));
        } // comment

      private:
        LexicalHandlerT* lexicalHandler_;
    }; // class LexicalHandlerAdaptor

    class DeclHandlerAdaptor : public XERCES_CPP_NAMESPACE::DeclHandler 
    {
      public:
        DeclHandlerAdaptor() : declHandler_(0) { }
        virtual ~DeclHandlerAdaptor() { }

        void setDeclHandler(DeclHandlerT& handler) { declHandler_ = &handler; }
        DeclHandlerT* getDeclHandler() const { return declHandler_; }

        virtual void elementDecl(const XMLCh* const name,
                                 const XMLCh* const model)
        {
          if(declHandler_)
            declHandler_->elementDecl(SA::makeStringT(name),
                                      SA::makeStringT(model));
        } // elementDecl

        virtual void attributeDecl(const XMLCh* const elementName,
                                   const XMLCh* const attributeName,
                                   const XMLCh* const type,
                                   const XMLCh* const valueDefault,
                                   const XMLCh* const value)
        {
          if(declHandler_)
          {
            const XMLCh* tmpValueDefault = valueDefault;
            if (tmpValueDefault == XERCES_CPP_NAMESPACE::XMLUni::fgNullString)
              tmpValueDefault = 0;
            declHandler_->attributeDecl(SA::makeStringT(elementName),
                                        SA::makeStringT(attributeName),
                                        SA::makeStringT(type),
                                        SA::makeStringT(tmpValueDefault),
                                        SA::makeStringT(value));
          }
        } // attributeDecl

        virtual void internalEntityDecl(const XMLCh* const name,
                                        const XMLCh* const value)
        {
          if(declHandler_)
            declHandler_->internalEntityDecl(SA::makeStringT(name),
                                             SA::makeStringT(value));
        } // internalEntityDecl

        virtual void externalEntityDecl(const XMLCh* const name,
                                        const XMLCh* const publicId,
                                        const XMLCh* const systemId)
        {
          if(declHandler_)
            declHandler_->externalEntityDecl(SA::makeStringT(name),
                                             SA::makeStringT(publicId),
                                             SA::makeStringT(systemId));
        } // externalEntityDecl

      private:
        DeclHandlerT* declHandler_;
    }; // class DeclHandlerAdaptor

    class IStreamAdaptor : public XERCES_CPP_NAMESPACE::BinInputStream
    {
      public:
        IStreamAdaptor(IStreamHandle istream)
        :   istream_(istream),
            curPos_(0)
        { }

        virtual ~IStreamAdaptor() { }

        virtual unsigned int curPos() const
        {
          return curPos_;
        } // curPos

        virtual unsigned int readBytes(XMLByte* const toFill,
                                       const unsigned int maxToRead)
        {
          istream_.get()->read(reinterpret_cast<char*>(toFill), maxToRead);
          curPos_ += istream_.get()->gcount();
          return istream_.get()->gcount();
        } // readBytes

      private:
        IStreamHandle istream_;
        unsigned int curPos_;
    }; // IStreamAdaptor

    class InputSourceAdaptor : public XERCES_CPP_NAMESPACE::InputSource
    {
      public:
        InputSourceAdaptor(const InputSourceT& source) 
        :   inputSource_(source) 
        { 
            xerces_string_janitor<XMLCh> wSystemId(SA::asXMLChString(source.getSystemId()));
            xerces_string_janitor<XMLCh> wPublicId(SA::asXMLChString(source.getPublicId()));
            setSystemId(wSystemId.get());
            setPublicId(wPublicId.get());
        } // InputSourceAdaptor
        virtual ~InputSourceAdaptor() { }

        virtual XERCES_CPP_NAMESPACE::BinInputStream* makeStream() const
        {
          return new IStreamAdaptor(inputSource_.getByteStreamHandle());
        } // makeStream
     
      private:
        // This is a local copy, so we do not rely on someone else keeping
        // their InputSourceT around for us.
        InputSourceT inputSource_;
    }; // class InputSourceAdaptor

    class XercesXMLPScanToken : public ::SAX::XMLPScanTokenParserImpl
    {
      public:
        XERCES_CPP_NAMESPACE::XMLPScanToken token_;
    };

    /////////////////////////////////////////////
    // Member variables
    std::auto_ptr<XercesImpl::xerces_initializer> initializer_;
    XERCES_CPP_NAMESPACE::SAX2XMLReader* xerces_;
    ContentHandlerAdaptor contentHandlerAdaptor_;
    EntityResolverAdaptor entityResolverAdaptor_;
    DTDHandlerAdaptor dtdHandlerAdaptor_;
    ErrorHandlerAdaptor errorHandlerAdaptor_;
    LexicalHandlerAdaptor lexicalHandlerAdaptor_;
    DeclHandlerAdaptor declHandlerAdaptor_;
    SAX::XercesPropertyNames<string_type, string_adaptorT> properties_;

    // Property values to return by reference
    string_type externalSchemaLocation_;
    string_type externalNoNamespaceSchemaLocation_;
}; // class xerces_wrapper

template<class string_type, class T0, class T1>
xerces_wrapper<string_type, T0, T1>::xerces_wrapper()
{
  try
  {
    std::auto_ptr<XercesImpl::xerces_initializer> init(new XercesImpl::xerces_initializer());
    initializer_ = init;
  }
  catch(const XERCES_CPP_NAMESPACE::XMLException& toCatch)
  {
    string_type s = SA::makeStringT(toCatch.getMessage());
    throw SAX::SAXException(SA::asStdString(s));
  } // catch

  xerces_ = XERCES_CPP_NAMESPACE::XMLReaderFactory::createXMLReader();

  xerces_->setContentHandler(&contentHandlerAdaptor_);
  xerces_->setEntityResolver(&entityResolverAdaptor_);
  xerces_->setDTDHandler(&dtdHandlerAdaptor_);
  xerces_->setErrorHandler(&errorHandlerAdaptor_);
  xerces_->setLexicalHandler(&lexicalHandlerAdaptor_);
  xerces_->setDeclarationHandler(&declHandlerAdaptor_);
} // xerces_wrapper

template<class string_type, class T0, class T1>
xerces_wrapper<string_type, T0, T1>::~xerces_wrapper()
{
  delete xerces_;
} // ~xerces_wrapper

template<class string_type, class T0, class T1>
bool xerces_wrapper<string_type, T0, T1>::getFeature(const string_type& name) const
{
  try 
  {
    xerces_string_janitor<XMLCh> wName(SA::asXMLChString(name));
    return xerces_->getFeature(wName.get());
  } // try
  catch(XERCES_CPP_NAMESPACE::SAXNotSupportedException& e) 
  {
    throw SAX::SAXNotSupportedException(SA::asStdString(SA::makeStringT(e.getMessage())));
  } // catch(SAXNotSupportedException& e)
  catch(XERCES_CPP_NAMESPACE::SAXNotRecognizedException& e) 
  {
    throw SAX::SAXNotRecognizedException(SA::asStdString(SA::makeStringT(e.getMessage())));
  } // catch(SAXNotRecognizedException& e)
} // getFeature

template<class string_type, class T0, class T1>
void xerces_wrapper<string_type, T0, T1>::setFeature(const string_type& name, bool value)
{
  try 
  {
    xerces_string_janitor<XMLCh> wName(SA::asXMLChString(name));
    xerces_->setFeature(wName.get(), value);
  } // try
  catch(XERCES_CPP_NAMESPACE::SAXNotSupportedException& e) 
  {
    throw SAX::SAXNotSupportedException(SA::asStdString(SA::makeStringT(e.getMessage())));
  } // catch(SAXNotSupportedException& e)
  catch(XERCES_CPP_NAMESPACE::SAXNotRecognizedException& e) 
  {
    throw SAX::SAXNotRecognizedException(SA::asStdString(SA::makeStringT(e.getMessage())));
  } // catch(SAXNotRecognizedException& e)
} // setFeature

template<class string_type, class T0, class T1>
#ifndef ARABICA_VS6_WORKAROUND
std::auto_ptr<typename SAX::basic_XMLReader<string_type>::PropertyBase> xerces_wrapper<string_type, T0, T1>::doGetProperty(const string_type& name)
#else
std::auto_ptr<SAX::basic_XMLReader<string_type>::PropertyBase> xerces_wrapper<string_type, T0, T1>::doGetProperty(const string_type& name)
#endif
{
  if(name == properties_.lexicalHandler)
  {
    typedef typename SAX::basic_XMLReader<string_type>::template Property<LexicalHandlerT *> Prop;
    Prop *prop = new Prop(lexicalHandlerAdaptor_.getLexicalHandler());
#ifndef ARABICA_VS6_WORKAROUND
    return std::auto_ptr<typename base::PropertyBase>(prop);
#else
    return std::auto_ptr<base::PropertyBase>(prop);
#endif
  }
  if(name == properties_.declHandler)
  {
    typedef typename SAX::basic_XMLReader<string_type>::template Property<DeclHandlerT*> Prop;
    Prop* prop = new Prop(declHandlerAdaptor_.getDeclHandler());
#ifndef ARABICA_VS6_WORKAROUND
    return std::auto_ptr<typename base::PropertyBase>(prop);
#else
    return std::auto_ptr<base::PropertyBase>(prop);
#endif
  }
  if (name == properties_.externalSchemaLocation)
  {
    typedef typename SAX::basic_XMLReader<string_type>::template Property<string_type&> StringPropertyType;

    XMLCh* xercesExternalSchemaLocation = 
        static_cast<XMLCh*>(xerces_->getProperty(
            XERCES_CPP_NAMESPACE::XMLUni::fgXercesSchemaExternalSchemaLocation));

    externalSchemaLocation_ = SA::makeStringT(xercesExternalSchemaLocation);
#ifndef ARABICA_VS6_WORKAROUND
    std::auto_ptr<typename base::PropertyBase> toReturn(new StringPropertyType(externalSchemaLocation_));
#else
    std::auto_ptr<base::PropertyBase> toReturn(new StringPropertyType(externalSchemaLocation_));
#endif
#ifdef SAXXERCES_DEBUG
    std::cerr << "Returning " << typeid(toReturn)
              << "(*(" << typeid(*toReturn.get()) << ")" 
              <<  toReturn.get() << ")" 
              << " containing value [" << externalSchemaLocation_ << "]"
              <<  std::endl;
#endif
    return toReturn;
  }
  if (name == properties_.externalNoNamespaceSchemaLocation)
  {
    typedef typename SAX::basic_XMLReader<string_type>::template Property<string_type&> StringPropertyType;

    XMLCh* xercesExternalNoNamespaceSchemaLocation = 
        static_cast<XMLCh*>(xerces_->getProperty(
            XERCES_CPP_NAMESPACE::XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation));

    externalNoNamespaceSchemaLocation_ = SA::makeStringT(xercesExternalNoNamespaceSchemaLocation);
#ifndef ARABICA_VS6_WORKAROUND
    return std::auto_ptr<typename base::PropertyBase>(new StringPropertyType(externalNoNamespaceSchemaLocation_));
#else
    return std::auto_ptr<base::PropertyBase>(new StringPropertyType(externalNoNamespaceSchemaLocation_));
#endif
  }
  throw SAX::SAXNotRecognizedException("Property not recognized ");    
} // doGetProperty

template<class string_type, class T0, class T1>
#ifndef ARABICA_VS6_WORKAROUND
void xerces_wrapper<string_type, T0, T1>::doSetProperty(const string_type& name, std::auto_ptr<typename base::PropertyBase> value)
#else
void xerces_wrapper<string_type, T0, T1>::doSetProperty(const string_type& name, std::auto_ptr<base::PropertyBase> value)
#endif
{
  if(name == properties_.lexicalHandler)
  {
    Property<SAX::basic_LexicalHandler<stringT>&>* prop = dynamic_cast<Property<SAX::basic_LexicalHandler<stringT>&>*>(value.get());

    if(!prop)
      throw std::runtime_error("bad_cast: Property LexicalHandler is wrong type, should be SAX::LexicalHandler&");

    lexicalHandlerAdaptor_.setLexicalHandler(prop->get());
    return;
  } // if ...

  if(name == properties_.declHandler)
  {
    Property<SAX::basic_DeclHandler<stringT>&>* prop = dynamic_cast<Property<SAX::basic_DeclHandler<stringT>&>*>(value.get());

    if(!prop)
      throw std::runtime_error("bad_cast: Property DeclHandler is wrong type, should be SAX::DeclHandler&");

    declHandlerAdaptor_.setDeclHandler(prop->get());
    return;
  } // if ...

  if (name == properties_.externalSchemaLocation)
  {
    typename base::PropertyBase* propBase = value.get();
#ifdef SAXXERCES_DEBUG
    std::cerr << "doSetProperty(externalSchemaLocation, &("
              << typeid(*propBase) << "))" << std::endl;
#endif
    typedef typename base::template Property<string_type&> propertyType;
    propertyType* prop = dynamic_cast<propertyType*>(propBase);
#ifdef SAXXERCES_DEBUG
    std::cerr << "    Extracted property to " << typeid(prop)
              << "(" << prop << ")" << std::endl;
#endif
    if (prop)
    {
      externalSchemaLocation_ = prop->get();
#ifdef SAXXERCES_DEBUG
      std::cerr << "    Setting property to " << externalSchemaLocation_ << std::endl;
#endif
      xerces_string_janitor<XMLCh> toDelete(SA::asXMLChString(externalSchemaLocation_));
      xerces_->setProperty(XERCES_CPP_NAMESPACE::XMLUni::fgXercesSchemaExternalSchemaLocation, const_cast<XMLCh*>(toDelete.get()));
    }
    else
    {
      throw SAXNotSupportedException("Property ExternalShemaLocation is wrong type, should be string_type");
    }
    return;
  }

  if (name == properties_.externalNoNamespaceSchemaLocation)
  {
    typename base::PropertyBase* propBase = value.get();
    typedef typename base::template Property<string_type&> propertyType;
#ifdef SAXXERCES_DEBUG
    std::cerr << "doSetProperty(externalNoNamespaceSchemaLocation, &("
              << typeid(*propBase) << "))" << std::endl;
#endif
    propertyType* prop = dynamic_cast<propertyType*>(propBase);
#ifdef SAXXERCES_DEBUG
    std::cerr << "    Extracted property to " << typeid(prop)
              << "(" << prop << ")" << std::endl;
#endif
    if (prop)
    {
      externalNoNamespaceSchemaLocation_ = prop->get();
#ifdef SAXXERCES_DEBUG
      std::cerr << "    Setting property to " << externalNoNamespaceSchemaLocation_ << std::endl;
#endif
      xerces_string_janitor<XMLCh> toDelete(SA::asXMLChString(externalNoNamespaceSchemaLocation_));
      xerces_->setProperty(XERCES_CPP_NAMESPACE::XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation, const_cast<XMLCh*>(toDelete.get()));
    }
    else
    {
      throw SAXNotSupportedException("Property ExternalNoNamespaceSchemaLocation is wrong type, should be string_type");
    }
    return;
  }

  throw SAX::SAXNotRecognizedException("Property not recognized ");    
}


template<class string_type, class T0, class T1>
void xerces_wrapper<string_type, T0, T1>::parse(InputSourceT& source)
{
  // if no stream is open, let Xerces deal with it
  if(source.getByteStream() == 0)
    xerces_->parse(SA::asStdString(source.getSystemId()).c_str()); 
  else
  {
    // we have to wrap everything up in a Xerces friendly way
    InputSourceAdaptor isAdaptor(source);
    xerces_->parse(isAdaptor);
  } // if ...
} // parse

template<class string_type, class T0, class T1>
bool xerces_wrapper<string_type, 
                    T0, T1>::parseFirst(InputSourceT& input,
                                        XMLPScanToken& toFill)
{
  std::auto_ptr<XercesXMLPScanToken> newToken(new XercesXMLPScanToken);
  // To store the result from Xerces parseFirst().
  bool result = false;
  if (input.getByteStream() == 0)
  {
    // We can just pass the systemID to parseFirst and let Xerces open the
    // stream.
    const char* systemID = SA::asStdString(input.getSystemId()).c_str();
    result = xerces_->parseFirst(systemID, newToken->token_);
  }
  else
  {
    // We need to wrap input in an InputSourceAdaptor.

    // isAdaptor can be a local variable because Xerces has to open any streams
    // within the parseFirst() call, so it should not need the
    // InputSourceAdaptor to hang around.
    InputSourceAdaptor isAdaptor(input);
    result = xerces_->parseFirst(isAdaptor, newToken->token_);
  }
  if (result) {
    // We need to explicitly convert to auto_ptr<base class>.
    std::auto_ptr<XMLPScanTokenParserImpl> toSet(newToken.release());
    toFill.setParserData(toSet);
  }
  return result;
} // parseFirst

template<class string_type, class T0, class T1>
bool xerces_wrapper<string_type, T0, T1>::parseNext(XMLPScanToken& token)
{
  // Extract the XercesXMLPScanToken from within token.
  XercesXMLPScanToken* xercesToken = 
          dynamic_cast<XercesXMLPScanToken*>(token.parserImpl());
  if (xercesToken == 0)
  {
    // If it failed to cast, we can simply refuse to parse, because this call
    // clearly isn't following a successful parseFirst().
    return false;
  }
  else
  {
    // Otherwise, just pass the token to Xerces.
    return xerces_->parseNext(xercesToken->token_);
  }
} // parseNext

template<class string_type, class T0, class T1>
void xerces_wrapper<string_type, T0, T1>::parseReset(XMLPScanToken& token)
{
  // Extract the XercesXMLPScanToken from within token.
  XercesXMLPScanToken* xercesToken = 
        dynamic_cast<XercesXMLPScanToken*>(token.parserImpl());
  // If it failed to cast, we can simply ignore it, because this call clearly
  // isn't following a successful parseFirst().
  if (xercesToken != 0)
  {
    xerces_->parseReset(xercesToken->token_);
  }
} // parseReset

} // namespace SAX

#endif
// end of file