# Hash Table (해시 테이블)

Pintos는 `lib/kernel/hash.c`에 hash table(해시 테이블) data structure(자료구조)를 제공합니다. 이를 사용하려면 header file(헤더 파일) '`lib/kernel/hash.h`'를 `#include <hash.h>`로 include(포함)해야 합니다. Pintos가 제공하는 code(코드) 중 hash table을 사용하는 것은 없으므로, 원하는 대로 그대로 사용하거나, 자신의 목적에 맞게 implementation(구현)을 수정하거나, 무시해도 됩니다.

virtual memory project(가상 메모리 프로젝트)의 대부분 implementation은 page(페이지)를 frame(프레임)으로 translate(변환)하기 위해 hash table을 사용합니다. hash table의 다른 사용처도 찾을 수 있습니다.

## Data Types (데이터 타입)

hash table은 struct hash로 표현됩니다.

* * *
    
    
    struct hash;
    

> 전체 hash table을 나타냅니다. `struct hash`의 actual member(실제 멤버)는 "opaque(불투명)"합니다. 즉 hash table을 사용하는 code는 `struct hash` member에 직접 접근해서는 안 되며, 그럴 필요도 없어야 합니다. 대신 hash table function(함수)과 macro(매크로)를 사용하세요.

hash table은 `struct hash_elem` type(타입)의 element(원소)에 대해 동작합니다.

* * *
    
    
    struct hash_elem;
    

> hash table에 포함하려는 structure(구조체)에 `struct hash_elem` member를 embed(내장)하세요. `struct hash`와 마찬가지로 `struct hash_elem`도 opaque합니다. hash table element를 다루는 모든 function은 실제 hash table element type에 대한 pointer(포인터)가 아니라 `struct hash_elem`에 대한 pointer를 받고 반환합니다.

hash table의 real element(실제 원소)가 주어졌을 때 `struct hash_elem`을 얻거나, 그 반대로 얻어야 하는 경우가 많습니다. hash table의 real element가 주어지면 `&` operator(연산자)를 사용하여 그 element의 `struct hash_elem` pointer를 얻을 수 있습니다. 반대 방향으로는 `hash_entry()` macro를 사용합니다.

* * *
    
    
    #define hash_entry (elem, type, member) { /* Omit details */ }
    

> `struct hash_elem`을 가리키는 pointer인 elem이 embed되어 있는 structure에 대한 pointer를 반환합니다. type에는 elem이 들어 있는 structure의 이름을, member에는 type 안에서 elem이 가리키는 member의 이름을 제공해야 합니다.
> 
> 예를 들어 `h`가 `h_elem`이라는 이름의 struct thread member(`struct hash_elem` type)를 가리키는 `struct hash_elem *` variable(변수)이라고 합시다. 그러면 `hash_entry` (`h, struct thread, h_elem`)은 `h`가 내부를 가리키고 있는 `struct thread`의 address(주소)를 산출합니다.

각 hash table element는 key(키)를 포함해야 합니다. key는 element를 identify(식별)하고 distinguish(구분)하는 data(데이터)이며, hash table 안의 element들 사이에서 unique(고유)해야 합니다. element는 unique할 필요가 없는 non-key data(키가 아닌 데이터)도 포함할 수 있습니다. element가 hash table 안에 있는 동안 key data는 변경되어서는 안 됩니다. 필요하다면 element를 hash table에서 remove(제거)하고 key를 modify(수정)한 뒤 element를 다시 insert(삽입)하세요.

각 hash table마다 key에 대해 동작하는 두 function, hash function(해시 함수)과 comparison function(비교 함수)을 작성해야 합니다. 이 function들은 다음 prototype(원형)과 일치해야 합니다.

* * *
    
    
    typedef unsigned hash_hash_func (const struct hash_elem *e, void *aux);
    

> element data의 hash(해시)를 unsigned int 범위의 값으로 반환합니다. element의 hash는 element key의 pseudo-random function(의사 난수 함수)이어야 합니다. element의 non-key data나 key 외의 non-constant data(상수가 아닌 데이터)에 의존해서는 안 됩니다. Pintos는 hash function의 적절한 basis(기반)로 다음 function들을 제공합니다.
> 
> `unsigned hash_bytes (const void *buf, size t *size)`
>
>> buf에서 시작하는 size bytes(바이트)에 대한 hash를 반환합니다. implementation은 32-bit word(워드)를 위한 general-purpose(범용) Fowler-Noll-Vo hash(`http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash`)입니다.
> 
> `unsigned hash_string (const char *s)`
>
>> null-terminated string(널 종료 문자열) s의 hash를 반환합니다.
> 
> `unsigned hash_int (int i)`
>
>> integer(정수) i의 hash를 반환합니다.
> 
> key가 적절한 type의 단일 data 조각이라면, hash function이 이 function 중 하나의 output(출력)을 직접 return(반환)하는 것이 합리적입니다. data 조각이 여러 개라면, 예를 들어 '^'(exclusive or, 배타적 OR) operator를 사용하여 이 function들을 여러 번 호출한 output을 combine(결합)할 수 있습니다. 마지막으로 이 function들을 완전히 무시하고 자신만의 hash function을 처음부터 작성할 수도 있지만, goal(목표)은 hash function design이 아니라 operating system kernel(운영체제 핵심부)을 만드는 것임을 기억하세요. aux에 대한 설명은 [Hash Auxiliary Data] section을 참조하세요.
> 
> `bool hash_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux)`
>
>> element a와 b에 저장된 key를 compare(비교)합니다. a가 b보다 작으면 true를, a가 b보다 크거나 같으면 false를 반환합니다. 두 element가 equal(같음)로 compare되면 같은 hash value(해시 값)를 가져야 합니다.
> 
> aux에 대한 설명은 [Hash Auxiliary Data] section을 참조하세요. hash와 comparison function 예시는 [Hash Table Example] section을 참조하세요. 몇몇 function은 세 번째 종류의 function pointer를 argument(인자)로 받습니다.
> 
> `void hash_action_func (struct hash_elem *element, void *aux)`
>
>> caller(호출자)가 선택한 어떤 action(동작)을 element에 대해 수행합니다. aux 설명은 [Hash Auxiliary Data] section을 참조하세요.

## Basic Functions (기본 함수)

이 function들은 hash table을 create(생성), destroy(파괴), inspect(검사)합니다.

* * *
    
    
    bool hash_init (struct hash *hash, hash_hash_func *hash_func,
                        hash_less_func *less_func, void *aux)
    

> hash를 hash table로 initialize(초기화)합니다. hash func는 hash function, less func는 comparison function, aux는 auxiliary data(보조 데이터)입니다. 성공하면 true, 실패하면 false를 반환합니다. `hash_init()`은 `malloc()`을 호출하며 memory(메모리)를 allocate(할당)할 수 없으면 fail(실패)합니다. aux 설명은 Hash Auxiliary Data를 참조하세요. aux는 보통 null pointer(널 포인터)입니다.

* * *
    
    
    void hash_clear (struct hash *hash, hash_action_func *action)
    

> `hash_init()`으로 이전에 initialize되어 있어야 하는 hash에서 모든 element를 remove합니다. action이 non-null이면 hash table의 각 element마다 한 번씩 호출됩니다. 이는 caller가 element가 사용하는 memory나 다른 resource(자원)를 deallocate(할당 해제)할 기회를 줍니다. 예를 들어 hash table element가 `malloc()`으로 dynamically allocated(동적 할당)되었다면 action은 element를 `free()`할 수 있습니다. `hash_clear()`는 action을 호출한 뒤 해당 hash element의 memory에 접근하지 않으므로 이는 safe(안전)합니다. 하지만 action은 `hash_insert()`나 `hash_delete()`처럼 hash table을 modify(수정)할 수 있는 function을 호출해서는 안 됩니다.

* * *
    
    
    void hash_destroy (struct hash *hash, hash_action_func *action);
    

> action이 non-null이면 `hash_clear()` 호출과 같은 semantics(의미)로 hash의 각 element에 대해 호출합니다. 그런 다음 hash가 보유한 memory를 free(해제)합니다. 이후에는 intervening(중간) `hash_init()` 호출이 없는 한 hash를 어떤 hash table function에도 전달해서는 안 됩니다.

* * *
    
    
    size_t hash_size (struct hash *hash);
    

> 현재 hash에 저장된 element 수를 반환합니다.

* * *
    
    
    bool hash_empty (struct hash *hash);
    

> hash가 현재 element를 하나도 포함하지 않으면 true를, 최소 하나의 element를 포함하면 false를 반환합니다.

## Search Functions (검색 함수)

각 function은 hash table에서 제공된 element와 equal하게 compare되는 element를 search(검색)합니다. search 성공 여부에 따라 새 element를 hash table에 insert하거나, 단순히 search result(결과)를 반환하는 등의 action을 수행합니다.

* * *
    
    
    struct hash_elem *hash_insert (struct hash *hash, struct hash elem *element);
    

> hash에서 element와 equal한 element를 search합니다. 없으면 element를 hash에 insert하고 null pointer를 반환합니다. table에 이미 element와 equal한 element가 있으면 hash를 modify하지 않고 그 element를 반환합니다.

* * *
    
    
    struct hash_elem *hash_replace (struct hash *hash, struct hash elem *element);
    

> element를 hash에 insert합니다. hash 안에 element와 equal한 element가 이미 있으면 remove됩니다. remove된 element를 반환하거나, hash가 element와 equal한 element를 포함하지 않았다면 null pointer를 반환합니다.
> 
> 반환된 element와 associated(연관)된 resource를 deallocate하는 책임은 caller에게 있습니다. 예를 들어 hash table element가 `malloc()`으로 dynamically allocated되었다면, 더 이상 필요하지 않을 때 caller가 element를 `free()`해야 합니다.

다음 function들에 전달되는 element는 hashing과 comparison 목적으로만 사용됩니다. 실제로 hash table에 insert되지 않습니다. 따라서 element 안의 key data만 initialize하면 되고, 다른 data는 사용되지 않습니다. element type의 instance(인스턴스)를 local variable(지역 변수)로 선언하고 key data를 initialize한 뒤, 그 `struct hash_elem`의 address를 `hash_find()`나 `hash_delete()`에 전달하는 것이 자주 합리적입니다. 예시는 Hash Table Example을 참조하세요. 큰 structure는 local variable로 allocate해서는 안 됩니다. 자세한 내용은 [struct thread](0_threads.md)를 참조하세요.

* * *
    
    
    struct hash_elem *hash_find (struct hash *hash, struct hash elem *element);
    

> hash에서 element와 equal한 element를 search합니다. 찾으면 해당 element를, 없으면 null pointer를 반환합니다.

* * *
    
    
    struct hash_elem *hash_delete (struct hash *hash, struct hash elem *element);
    

> hash에서 element와 equal한 element를 search합니다. 찾으면 hash에서 remove하고 반환합니다. 없으면 null pointer를 반환하고 hash는 unchanged(변경되지 않음) 상태로 남습니다.
> 
> 반환된 element와 associated된 resource를 deallocate하는 책임은 caller에게 있습니다. 예를 들어 hash table element가 `malloc()`으로 dynamically allocated되었다면, 더 이상 필요하지 않을 때 caller가 element를 `free()`해야 합니다.

## Iteration Functions (반복 함수)

이 function들은 hash table의 element를 iterate(순회)할 수 있게 합니다. 두 interface(인터페이스)가 제공됩니다. 첫 번째는 각 element에 대해 action을 수행하는 hash_action_func를 작성하여 제공해야 합니다.

* * *
    
    
    void hash_apply (struct hash *hash, hash action func *action);
    

> hash의 각 element에 대해 action을 arbitrary order(임의 순서)로 한 번씩 호출합니다. action은 `hash_insert()`나 `hash_delete()`처럼 hash table을 modify할 수 있는 function을 호출해서는 안 됩니다. action은 element의 key data를 modify해서는 안 되지만, 다른 data는 modify할 수 있습니다.

두 번째 interface는 "iterator(반복자)" data type에 기반합니다. 관용적으로 iterator는 다음과 같이 사용합니다.

* * *
    
    
    struct hash_iterator i;
    hash_first (&i, h);
    while (hash_next (&i)) {
        struct foo *f = hash_entry (hash_cur (&i), struct foo, elem);
        . . . do something with f . . .
    }
    

* * *
    
    
    struct hash_iterator;
    

> hash table 안의 position(위치)을 나타냅니다. `hash_insert()`나 `hash_delete()`처럼 hash table을 modify할 수 있는 function을 호출하면 해당 hash table 안의 모든 iterator가 invalid(무효)해집니다.
> 
> struct hash와 struct hash_elem처럼 struct hash_elem은 opaque합니다.

* * *
    
    
    void hash_first (struct hash iterator *iterator, struct hash *hash);
    

> iterator를 hash의 첫 element 바로 전으로 initialize합니다.
    
    
    struct hash_elem *hash_next (struct hash iterator *iterator);
    

> iterator를 hash의 다음 element로 advance(진행)하고 그 element를 반환합니다. 남은 element가 없으면 null pointer를 반환합니다. `hash_next()`가 iterator에 대해 null을 반환한 뒤 다시 호출하면 undefined behavior(정의되지 않은 동작)를 일으킵니다.

* * *
    
    
    struct hash_elem *hash_cur (struct hash iterator *iterator);
    

> iterator에 대해 `hash_next()`가 가장 최근에 반환한 value(값)를 반환합니다. iterator에 대해 `hash_first()`가 호출된 뒤 첫 `hash_next()`가 호출되기 전에는 undefined behavior를 일으킵니다.

## Hash Table Example (해시 테이블 예시)

hash table에 넣고 싶은 `struct page`라는 structure가 있다고 합시다. 먼저 `struct page`가 `struct hash_elem` member를 포함하도록 define(정의)합니다.
    
    
    struct page {
        struct hash_elem hash_elem; /* Hash table element. */
        void *addr; /* Virtual address. */
        /* . . . other members. . . */
    };
    

addr을 key로 사용하여 hash function과 comparison function을 작성합니다. pointer는 그 byte를 바탕으로 hash할 수 있고, pointer compare에는 `<` operator가 잘 동작합니다.
    
    
    /* Returns a hash value for page p. */
    unsigned
    page_hash (const struct hash_elem *p_, void *aux UNUSED) {
      const struct page *p = hash_entry (p_, struct page, hash_elem);
      return hash_bytes (&p->addr, sizeof p->addr);
    }
    
    
    
    /* Returns true if page a precedes page b. */
    bool
    page_less (const struct hash_elem *a_,
               const struct hash_elem *b_, void *aux UNUSED) {
      const struct page *a = hash_entry (a_, struct page, hash_elem);
      const struct page *b = hash_entry (b_, struct page, hash_elem);
    
      return a->addr < b->addr;
    }
    

(이 function들의 prototype에서 `UNUSED`를 사용하면 aux가 unused(사용되지 않음)라는 warning(경고)을 suppress(억제)합니다. `UNUSED`에 대한 정보는 Function and Parameter Attributes를 참조하세요. aux 설명은 Hash Auxiliary Data를 참조하세요.)

그런 다음 다음처럼 hash table을 create할 수 있습니다.
    
    
    struct hash pages;
    hash_init (&pages, page_hash, page_less, NULL);
    

이제 create한 hash table을 manipulate(조작)할 수 있습니다. `p`가 `struct page`를 가리키는 pointer라면 다음으로 hash table에 insert할 수 있습니다.
    
    
    hash_insert (&pages, &p->hash_elem);
    

pages가 같은 addr을 가진 page를 이미 contain(포함)할 가능성이 있다면 `hash_insert()`의 return value를 check해야 합니다.

hash table에서 element를 search하려면 `hash_find()`를 사용합니다. `hash_find()`는 compare 대상 element를 받으므로 약간의 setup이 필요합니다. 다음은 pages가 file scope(파일 범위)에 정의되어 있다고 가정하고, virtual address(가상 주소)를 바탕으로 page를 찾아 반환하는 function입니다.
    
    
    /* Returns the page containing the given virtual address, or a null pointer if no such page exists. */
    struct page *
    page_lookup (const void *address) {
      struct page p;
      struct hash_elem *e;
    
      p.addr = address;
      e = hash_find (&pages, &p.hash_elem);
      return e != NULL ? hash_entry (e, struct page, hash_elem) : NULL;
    }
    

여기서는 `struct page`가 꽤 작다고 가정하고 local variable로 allocate했습니다. 큰 structure는 local variable로 allocate해서는 안 됩니다.

비슷한 function이 `hash_delete()`를 사용하여 address로 page를 delete(삭제)할 수 있습니다.

## Auxiliary Data (보조 데이터)

위 example처럼 simple case(단순한 경우)에서는 aux parameter(매개변수)가 필요하지 않습니다. 이런 경우 `hash_init()`의 aux로 null pointer를 전달하고, hash function과 comparison function에 전달된 값을 무시하면 됩니다. aux parameter를 사용하지 않으면 compiler warning이 발생하지만, example처럼 `UNUSED` macro로 끄거나 그냥 무시할 수 있습니다.

***aux***는 hash table 안의 data에 대한 어떤 property(속성)가 constant(상수)이고 hashing 또는 comparison에 필요하지만 data item(데이터 항목) 자체에는 저장되어 있지 않을 때 유용합니다. 예를 들어 hash table의 item이 fixed-length string(고정 길이 문자열)이지만 item 자체가 그 fixed length를 나타내지 않는다면, length(길이)를 aux parameter로 전달할 수 있습니다.

## Synchronization (동기화)

hash table은 internal synchronization(내부 동기화)을 하지 않습니다. hash table function 호출을 synchronize(동기화)하는 것은 caller의 책임입니다. 일반적으로 `hash_find()`나 `hash_next()`처럼 hash table을 examine(검사)하지만 modify하지 않는 function은 여러 개가 동시에 execute(실행)될 수 있습니다. 하지만 이런 function은 `hash_insert()`나 `hash_delete()`처럼 주어진 hash table을 modify할 수 있는 function과 동시에 safely execute(안전하게 실행)될 수 없으며, 주어진 hash table을 modify할 수 있는 function 둘 이상도 동시에 safely execute될 수 없습니다.

hash table element 안의 data 접근을 synchronize하는 것도 caller의 책임입니다. 이 data에 대한 access(접근)를 어떻게 synchronize할지는 다른 data structure와 마찬가지로 design(설계)과 organization(구성)에 따라 달라집니다.
