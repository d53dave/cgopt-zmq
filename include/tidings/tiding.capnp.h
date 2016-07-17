// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: tiding.capnp

#ifndef CAPNP_INCLUDED_a516a3d712db6cf8_
#define CAPNP_INCLUDED_a516a3d712db6cf8_

#include <capnp/generated-header-support.h>

#if CAPNP_VERSION != 5003
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(f07c8ad405cfcf5d);
CAPNP_DECLARE_SCHEMA(e58f8ab6ff33a10e);
enum class Type_e58f8ab6ff33a10e: uint16_t {
  ERROR,
  HEARTBEAT,
  WORK_REQUEST,
  WORK_RESULTS,
  STATS,
};
CAPNP_DECLARE_ENUM(Type, e58f8ab6ff33a10e);
CAPNP_DECLARE_SCHEMA(e8ccb005ff482fd9);
CAPNP_DECLARE_SCHEMA(d19a4fb06ef0e6ac);
enum class ErrorType_d19a4fb06ef0e6ac: uint16_t {
  BUILD_ERROR,
  EXECUTION_ERROR,
  TIME_OUT_ERROR,
  COMMUNICATION_ERROR,
};
CAPNP_DECLARE_ENUM(ErrorType, d19a4fb06ef0e6ac);

}  // namespace schemas
}  // namespace capnp


struct Tiding {
  Tiding() = delete;

  class Reader;
  class Builder;
  class Pipeline;
  typedef ::capnp::schemas::Type_e58f8ab6ff33a10e Type;

  struct Error;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(f07c8ad405cfcf5d, 2, 4)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

struct Tiding::Error {
  Error() = delete;

  class Reader;
  class Builder;
  class Pipeline;
  typedef ::capnp::schemas::ErrorType_d19a4fb06ef0e6ac ErrorType;


  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(e8ccb005ff482fd9, 1, 1)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand = &schema->defaultBrand;
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class Tiding::Reader {
public:
  typedef Tiding Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand);
  }
#endif  // !CAPNP_LITE

  inline bool hasId() const;
  inline  ::capnp::Text::Reader getId() const;

  inline bool hasSender() const;
  inline  ::capnp::Text::Reader getSender() const;

  inline  ::uint64_t getTimestamp() const;

  inline  ::Tiding::Type getType() const;

  inline bool hasError() const;
  inline  ::Tiding::Error::Reader getError() const;

  inline bool hasPayload() const;
  inline  ::capnp::List< ::capnp::Data>::Reader getPayload() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Tiding::Builder {
public:
  typedef Tiding Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline bool hasId();
  inline  ::capnp::Text::Builder getId();
  inline void setId( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initId(unsigned int size);
  inline void adoptId(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownId();

  inline bool hasSender();
  inline  ::capnp::Text::Builder getSender();
  inline void setSender( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initSender(unsigned int size);
  inline void adoptSender(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownSender();

  inline  ::uint64_t getTimestamp();
  inline void setTimestamp( ::uint64_t value);

  inline  ::Tiding::Type getType();
  inline void setType( ::Tiding::Type value);

  inline bool hasError();
  inline  ::Tiding::Error::Builder getError();
  inline void setError( ::Tiding::Error::Reader value);
  inline  ::Tiding::Error::Builder initError();
  inline void adoptError(::capnp::Orphan< ::Tiding::Error>&& value);
  inline ::capnp::Orphan< ::Tiding::Error> disownError();

  inline bool hasPayload();
  inline  ::capnp::List< ::capnp::Data>::Builder getPayload();
  inline void setPayload( ::capnp::List< ::capnp::Data>::Reader value);
  inline void setPayload(::kj::ArrayPtr<const  ::capnp::Data::Reader> value);
  inline  ::capnp::List< ::capnp::Data>::Builder initPayload(unsigned int size);
  inline void adoptPayload(::capnp::Orphan< ::capnp::List< ::capnp::Data>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::capnp::Data>> disownPayload();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Tiding::Pipeline {
public:
  typedef Tiding Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

  inline  ::Tiding::Error::Pipeline getError();
private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class Tiding::Error::Reader {
public:
  typedef Error Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand);
  }
#endif  // !CAPNP_LITE

  inline  ::Tiding::Error::ErrorType getErrorType() const;

  inline bool hasErrorMessage() const;
  inline  ::capnp::Text::Reader getErrorMessage() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Tiding::Error::Builder {
public:
  typedef Error Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::Tiding::Error::ErrorType getErrorType();
  inline void setErrorType( ::Tiding::Error::ErrorType value);

  inline bool hasErrorMessage();
  inline  ::capnp::Text::Builder getErrorMessage();
  inline void setErrorMessage( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initErrorMessage(unsigned int size);
  inline void adoptErrorMessage(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownErrorMessage();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Tiding::Error::Pipeline {
public:
  typedef Error Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

// =======================================================================================

inline bool Tiding::Reader::hasId() const {
  return !_reader.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline bool Tiding::Builder::hasId() {
  return !_builder.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Tiding::Reader::getId() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _reader.getPointerField(0 * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Tiding::Builder::getId() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}
inline void Tiding::Builder::setId( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(
      _builder.getPointerField(0 * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Tiding::Builder::initId(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(
      _builder.getPointerField(0 * ::capnp::POINTERS), size);
}
inline void Tiding::Builder::adoptId(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(
      _builder.getPointerField(0 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Tiding::Builder::disownId() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}

inline bool Tiding::Reader::hasSender() const {
  return !_reader.getPointerField(1 * ::capnp::POINTERS).isNull();
}
inline bool Tiding::Builder::hasSender() {
  return !_builder.getPointerField(1 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Tiding::Reader::getSender() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _reader.getPointerField(1 * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Tiding::Builder::getSender() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _builder.getPointerField(1 * ::capnp::POINTERS));
}
inline void Tiding::Builder::setSender( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(
      _builder.getPointerField(1 * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Tiding::Builder::initSender(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(
      _builder.getPointerField(1 * ::capnp::POINTERS), size);
}
inline void Tiding::Builder::adoptSender(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(
      _builder.getPointerField(1 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Tiding::Builder::disownSender() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(
      _builder.getPointerField(1 * ::capnp::POINTERS));
}

inline  ::uint64_t Tiding::Reader::getTimestamp() const {
  return _reader.getDataField< ::uint64_t>(
      0 * ::capnp::ELEMENTS);
}

inline  ::uint64_t Tiding::Builder::getTimestamp() {
  return _builder.getDataField< ::uint64_t>(
      0 * ::capnp::ELEMENTS);
}
inline void Tiding::Builder::setTimestamp( ::uint64_t value) {
  _builder.setDataField< ::uint64_t>(
      0 * ::capnp::ELEMENTS, value);
}

inline  ::Tiding::Type Tiding::Reader::getType() const {
  return _reader.getDataField< ::Tiding::Type>(
      4 * ::capnp::ELEMENTS);
}

inline  ::Tiding::Type Tiding::Builder::getType() {
  return _builder.getDataField< ::Tiding::Type>(
      4 * ::capnp::ELEMENTS);
}
inline void Tiding::Builder::setType( ::Tiding::Type value) {
  _builder.setDataField< ::Tiding::Type>(
      4 * ::capnp::ELEMENTS, value);
}

inline bool Tiding::Reader::hasError() const {
  return !_reader.getPointerField(2 * ::capnp::POINTERS).isNull();
}
inline bool Tiding::Builder::hasError() {
  return !_builder.getPointerField(2 * ::capnp::POINTERS).isNull();
}
inline  ::Tiding::Error::Reader Tiding::Reader::getError() const {
  return ::capnp::_::PointerHelpers< ::Tiding::Error>::get(
      _reader.getPointerField(2 * ::capnp::POINTERS));
}
inline  ::Tiding::Error::Builder Tiding::Builder::getError() {
  return ::capnp::_::PointerHelpers< ::Tiding::Error>::get(
      _builder.getPointerField(2 * ::capnp::POINTERS));
}
#if !CAPNP_LITE
inline  ::Tiding::Error::Pipeline Tiding::Pipeline::getError() {
  return  ::Tiding::Error::Pipeline(_typeless.getPointerField(2));
}
#endif  // !CAPNP_LITE
inline void Tiding::Builder::setError( ::Tiding::Error::Reader value) {
  ::capnp::_::PointerHelpers< ::Tiding::Error>::set(
      _builder.getPointerField(2 * ::capnp::POINTERS), value);
}
inline  ::Tiding::Error::Builder Tiding::Builder::initError() {
  return ::capnp::_::PointerHelpers< ::Tiding::Error>::init(
      _builder.getPointerField(2 * ::capnp::POINTERS));
}
inline void Tiding::Builder::adoptError(
    ::capnp::Orphan< ::Tiding::Error>&& value) {
  ::capnp::_::PointerHelpers< ::Tiding::Error>::adopt(
      _builder.getPointerField(2 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::Tiding::Error> Tiding::Builder::disownError() {
  return ::capnp::_::PointerHelpers< ::Tiding::Error>::disown(
      _builder.getPointerField(2 * ::capnp::POINTERS));
}

inline bool Tiding::Reader::hasPayload() const {
  return !_reader.getPointerField(3 * ::capnp::POINTERS).isNull();
}
inline bool Tiding::Builder::hasPayload() {
  return !_builder.getPointerField(3 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::capnp::Data>::Reader Tiding::Reader::getPayload() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Data>>::get(
      _reader.getPointerField(3 * ::capnp::POINTERS));
}
inline  ::capnp::List< ::capnp::Data>::Builder Tiding::Builder::getPayload() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Data>>::get(
      _builder.getPointerField(3 * ::capnp::POINTERS));
}
inline void Tiding::Builder::setPayload( ::capnp::List< ::capnp::Data>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Data>>::set(
      _builder.getPointerField(3 * ::capnp::POINTERS), value);
}
inline void Tiding::Builder::setPayload(::kj::ArrayPtr<const  ::capnp::Data::Reader> value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Data>>::set(
      _builder.getPointerField(3 * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::capnp::Data>::Builder Tiding::Builder::initPayload(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Data>>::init(
      _builder.getPointerField(3 * ::capnp::POINTERS), size);
}
inline void Tiding::Builder::adoptPayload(
    ::capnp::Orphan< ::capnp::List< ::capnp::Data>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Data>>::adopt(
      _builder.getPointerField(3 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::capnp::Data>> Tiding::Builder::disownPayload() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::capnp::Data>>::disown(
      _builder.getPointerField(3 * ::capnp::POINTERS));
}

inline  ::Tiding::Error::ErrorType Tiding::Error::Reader::getErrorType() const {
  return _reader.getDataField< ::Tiding::Error::ErrorType>(
      0 * ::capnp::ELEMENTS);
}

inline  ::Tiding::Error::ErrorType Tiding::Error::Builder::getErrorType() {
  return _builder.getDataField< ::Tiding::Error::ErrorType>(
      0 * ::capnp::ELEMENTS);
}
inline void Tiding::Error::Builder::setErrorType( ::Tiding::Error::ErrorType value) {
  _builder.setDataField< ::Tiding::Error::ErrorType>(
      0 * ::capnp::ELEMENTS, value);
}

inline bool Tiding::Error::Reader::hasErrorMessage() const {
  return !_reader.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline bool Tiding::Error::Builder::hasErrorMessage() {
  return !_builder.getPointerField(0 * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Tiding::Error::Reader::getErrorMessage() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _reader.getPointerField(0 * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Tiding::Error::Builder::getErrorMessage() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}
inline void Tiding::Error::Builder::setErrorMessage( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(
      _builder.getPointerField(0 * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Tiding::Error::Builder::initErrorMessage(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(
      _builder.getPointerField(0 * ::capnp::POINTERS), size);
}
inline void Tiding::Error::Builder::adoptErrorMessage(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(
      _builder.getPointerField(0 * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Tiding::Error::Builder::disownErrorMessage() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(
      _builder.getPointerField(0 * ::capnp::POINTERS));
}


#endif  // CAPNP_INCLUDED_a516a3d712db6cf8_
