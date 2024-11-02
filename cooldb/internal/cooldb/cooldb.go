package cooldb

import (
	"sync"
)

type CoolDB struct {
	data map[string]string
	mu   sync.RWMutex
}

func New() *CoolDB {
	return &CoolDB{
		data: make(map[string]string, 1000), // Pre-allocate with reasonable capacity
	}
}

func (db *CoolDB) Set(key string, value string) {
	db.mu.Lock()
	db.data[key] = value
	db.mu.Unlock()
}

func (db *CoolDB) Get(key string) (string, bool) {
	db.mu.RLock()
	value, exists := db.data[key]
	db.mu.RUnlock()
	return value, exists
}

func (db *CoolDB) List() []string {
	db.mu.RLock()
	keys := make([]string, 0, len(db.data))
	for k := range db.data {
		keys = append(keys, k)
	}
	db.mu.RUnlock()
	return keys
}

func (db *CoolDB) Status() string {
	db.mu.RLock()
	status := "OK: " + string(len(db.data)) + " keys"
	db.mu.RUnlock()
	return status
}
