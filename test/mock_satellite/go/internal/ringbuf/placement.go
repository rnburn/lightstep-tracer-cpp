package ringbuf

type Placement struct {
  Data1 []byte
  Data2 []byte
}

func (placement *Placement) Size() int {
  return len(placement.Data1) + len(placement.Data2)
}
