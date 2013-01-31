/* protobuf config.h for MSVC.  On other platforms, this is generated
 * automatically by autoheader / autoconf / configure. */

/* the location of <hash_map> */
#define HASH_MAP_H <tr1/unordered_map>

#define HASH_NAMESPACE tr1

/* the location of <hash_set> */
// #define HASH_SET_H <hash_set>
#undef HASH_SET_H

/* define if the compiler has hash_map */
#define HAVE_HASH_MAP 1

/* define if the compiler has hash_set */
#undef HAVE_HASH_SET

/* define if you want to use zlib.  See readme.txt for additional
 * requirements. */
// #define HAVE_ZLIB 1
