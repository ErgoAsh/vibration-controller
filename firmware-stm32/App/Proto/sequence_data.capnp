@0xfcf97e9f5618cdcb;

struct Point {
    index @0 :UInt32;
    x @1: Float32;
    v @2: Float32;
    a @3: Float32;
    u @4: UInt8;
}

struct SequenceData {
    data @0 :List(Point);
}