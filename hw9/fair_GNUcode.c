int __pthread_spin_lock (pthread_spinlock_t *lock) {
    int val=0;
    if (__glibc_likely(atomic_exchange_acquire(lock,1) ==0))//acquire: 讀取訊號。假設spinlock會成功，回傳lock舊的值，若lock=0，有lock了，就回傳0結束；若lock=1，交換兩個值，if不成立，執行下面do while
        return 0;
    do {
        do {    
            //TODO Back-off（讓大家隨機地等待一段時間）
            atomic_spin_nop();  //目的不希望CPU太熱(燙是因為不斷測試)
            val= atomic_load_relaxed(lock); //載入lock值
        } while (val != 0);   //value為0，跳出，要跟下面lock比對
    } while (!atomic_compare_exchange_weak_acquire(lock, &val, 1));    //weak: 可能所有人都失敗。比較慢，所以需要17到19行，效能會變好。比lock和val都為0，其中一個lock被設定成1，再鎖上，會成功的跳出do while
    return 0;
}
int __pthread_spin_unlock(pthread_spinlock_t *lock) {
    atomic_store_release(lock, 0);   //release: 釋放訊號。把lock值設定成0
    return 0;
}