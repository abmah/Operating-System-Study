def copy_file():
    source = input("Enter source file path: ").strip()
    dest = input("Enter destination file path: ").strip()

    try:
        with open(source, "rb") as src, open(dest, "wb") as dst:
            while True:
                chunk = src.read(1024)  # read 1 KB at a time
                if not chunk:
                    break
                dst.write(chunk)
        print("File copied successfully!")
    except FileNotFoundError:
        print("Error: Source file does not exist.")
    except PermissionError:
        print("Error: Permission denied.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    copy_file()
