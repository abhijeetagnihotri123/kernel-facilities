def main():
    driver_handle = open('/proc/LDD_PROC_MODULE')

    message = driver_handle.readline()

    print(f"Message from kernel module: {message}")
    driver_handle.close()
    return

if __name__ == "__main__":
    main()