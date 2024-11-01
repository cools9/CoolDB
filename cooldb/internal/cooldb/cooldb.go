package cooldb

import (
	"sync"
)

type CoolDB struct {
    keys   []string
    values []string
    mu     sync.RWMutex
}

func New() *CoolDB {
    return &CoolDB{
        keys:   make([]string, 0),
        values: make([]string, 0),
    }
}

func (db *CoolDB) Set(key string, value string) {
    db.mu.Lock()
    defer db.mu.Unlock()

    // Check if key exists
    for i, k := range db.keys {
        if k == key {
            db.values[i] = value
            return
        }
    }

    db.keys = append(db.keys, key)
    db.values = append(db.values, value)
}

func (db *CoolDB) Get(key string) (string, bool) {
    db.mu.RLock()
    defer db.mu.RUnlock()

    for i, k := range db.keys {
        if k == key {
            return db.values[i], true
        }
    }
    return "", false
}

func (db *CoolDB) List() []string {
    db.mu.RLock()
    defer db.mu.RUnlock()

    keysCopy := make([]string, len(db.keys))
    copy(keysCopy, db.keys)
    return keysCopy
}

func (db *CoolDB) Status() string {
    return "OK"
}
