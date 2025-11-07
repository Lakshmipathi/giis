# GIIS - Architecture Diagram

## Project Overview
**GIIS (gET iT i sAY)** is a text-based file recovery tool for Ext2/Ext3 file systems on Linux. It works by creating a snapshot of file metadata during installation, which is then used to recover deleted files.

---

## System Architecture

```mermaid
graph TB
    subgraph "User Interface Layer"
        CLI[Command Line Interface<br/>main.c]
    end

    subgraph "Core Business Logic"
        INIT[Initialization Module<br/>init.c]
        RECOVERY[Recovery Engine<br/>get_it_i_say.c]
        SEARCH[Search & Update<br/>searchnupdate.c]
    end

    subgraph "Data Access Layer"
        DIR[Directory Operations<br/>dir.c]
        FILE[File Operations<br/>file.c]
        INODE[Inode Operations<br/>inode.c]
        GROUP[Group Descriptors<br/>group.c]
    end

    subgraph "Filesystem Layer"
        FS[Ext2/Ext3 Filesystem<br/>/dev/sdaX]
        SUPER[Superblock]
        GD[Group Descriptors]
        INODES[Inode Table]
        BLOCKS[Data Blocks]
    end

    subgraph "Data Storage"
        DB_DIR[(Directory DB<br/>sys/dir)]
        DB_FILE[(File DB<br/>sys/file)]
        DB_SIND[(Single Indirect<br/>sys/sind)]
        DB_DIND[(Double Indirect<br/>sys/dind)]
        DB_SAMPLE[(Sample Data<br/>sys/sample)]
        CONFIG[Config File<br/>/etc/giis.conf]
    end

    subgraph "Recovery Output"
        RECOVERED[Recovered Files<br/>got_it/]
        UNREC[Unrecovered List<br/>unrecovered/]
        RESTORE[Restore Details<br/>restore_details.txt]
    end

    CLI --> INIT
    CLI --> RECOVERY
    CLI --> SEARCH

    INIT --> DIR
    INIT --> INODE
    INIT --> GROUP

    RECOVERY --> FILE
    RECOVERY --> INODE
    RECOVERY --> DB_FILE
    RECOVERY --> DB_SAMPLE

    SEARCH --> DIR
    SEARCH --> FILE
    SEARCH --> DB_DIR
    SEARCH --> DB_FILE

    DIR --> INODE
    DIR --> DB_DIR
    DIR --> DB_FILE

    FILE --> INODE
    FILE --> DB_FILE
    FILE --> DB_SIND
    FILE --> DB_DIND
    FILE --> DB_SAMPLE

    INODE --> FS
    GROUP --> FS

    FS --> SUPER
    FS --> GD
    FS --> INODES
    FS --> BLOCKS

    RECOVERY --> RECOVERED
    RECOVERY --> UNREC
    RECOVERY --> RESTORE

    INIT --> CONFIG
    SEARCH --> CONFIG

    style CLI fill:#e1f5ff
    style INIT fill:#fff4e1
    style RECOVERY fill:#ffe1e1
    style SEARCH fill:#e1ffe1
    style FS fill:#f0f0f0
    style RECOVERED fill:#d4edda
```

---

## Component Interaction Diagram

```mermaid
sequenceDiagram
    participant User
    participant main.c
    participant init.c
    participant dir.c
    participant file.c
    participant inode.c
    participant Database
    participant Filesystem

    Note over User,Filesystem: Installation Flow (giis -i)

    User->>main.c: giis -i
    main.c->>init.c: init()
    init.c->>Filesystem: search4fs()
    Filesystem-->>init.c: Device: /dev/sda1
    init.c->>Filesystem: search4super_block()
    Filesystem-->>init.c: Superblock data
    init.c->>Filesystem: search4group_desc()
    Filesystem-->>init.c: Group descriptors
    init.c->>Database: init_files()
    Database-->>init.c: Created sys/ directories

    main.c->>inode.c: find_inode_offset(root)
    inode.c->>Filesystem: Read inode 2
    Filesystem-->>inode.c: Root inode data

    main.c->>dir.c: show_dir("/")
    loop For each directory
        dir.c->>inode.c: read_inode()
        inode.c->>Filesystem: Read directory inode
        Filesystem-->>inode.c: Inode data
        dir.c->>file.c: record_file()
        file.c->>Database: Write file metadata
        dir.c->>Database: record_dir()
    end

    dir.c->>file.c: get_sample_data()
    file.c->>Filesystem: Read first 32 bytes
    Filesystem-->>file.c: Sample data
    file.c->>Database: Store sample data

    main.c-->>User: Installation complete

    Note over User,Filesystem: Recovery Flow (giis -g)

    User->>main.c: giis -g
    main.c->>file.c: Read FILE_INFO_FILE
    file.c->>Database: Load deleted files
    Database-->>file.c: List of deleted files
    file.c-->>main.c: Deleted file list

    main.c-->>User: Display deleted files
    User->>main.c: Select files to recover

    loop For each file
        main.c->>inode.c: verify_inode()
        inode.c->>Filesystem: Check if inode free
        Filesystem-->>inode.c: Inode status

        main.c->>file.c: read_sample_info_file()
        file.c->>Database: Get sample data
        Database-->>file.c: Original 32 bytes

        main.c->>Filesystem: Read current data
        Filesystem-->>main.c: Current data

        alt Data matches sample
            main.c->>main.c: get_it_i_say()
            main.c->>Filesystem: Read data blocks
            Filesystem-->>main.c: File data
            main.c->>Database: Write to got_it/
        else Data overwritten
            main.c-->>User: Offer giis dump
        end
    end

    main.c-->>User: Recovery complete
```

---

## Data Flow Diagram

```mermaid
flowchart TD
    A[User Input] --> B{Operation Mode}

    B -->|Install -i| C[Initialize System]
    B -->|Update -u| D[Update Database]
    B -->|Recover -g| E[Recover Files]
    B -->|List -l| F[List Deleted Files]
    B -->|Uninstall -U| G[Remove GIIS]

    C --> H[Scan Filesystem]
    H --> I[Read Superblock]
    H --> J[Read Group Descriptors]
    H --> K[Read Inodes]

    K --> L[Traverse Directories]
    L --> M[Record Directory Info]
    L --> N[Record File Info]
    N --> O[Extract Sample Data]

    M --> P[(Directory DB)]
    N --> Q[(File DB)]
    O --> R[(Sample DB)]

    D --> S[Read Config]
    S --> T[Scan Specified Dirs]
    T --> U{New/Modified?}
    U -->|Yes| N
    U -->|No| V[Skip]

    E --> W[Load File DB]
    W --> X[Filter Deleted Files]
    X --> Y{User Selection}

    Y --> Z[Verify Inode Status]
    Z --> AA{Inode Free?}

    AA -->|Yes| AB[Compare Sample Data]
    AB --> AC{Data Matches?}

    AC -->|Yes| AD[Read Data Blocks]
    AD --> AE[Direct Blocks 0-11]
    AD --> AF[Indirect Block 12]
    AD --> AG[Double Indirect 13]

    AE --> AH[Write to got_it/]
    AF --> AH
    AG --> AH

    AC -->|No| AI[Offer GIIS Dump]
    AI --> AJ{Force Recover?}
    AJ -->|Yes| AD
    AJ -->|No| AK[Add to Unrecovered]

    AA -->|No| AL[Cannot Recover]
    AL --> AK

    AH --> AM{Restore?}
    AM -->|Yes| AN[Restore to Original Location]
    AM -->|No| AO[Keep in got_it/]

    F --> W
    W --> AP[Display Deleted Files]

    G --> AQ[Remove System Files]
    AQ --> AR[Remove Cron Job]

    style A fill:#e1f5ff
    style C fill:#fff4e1
    style D fill:#ffe1e1
    style E fill:#e1ffe1
    style AH fill:#d4edda
    style AK fill:#f8d7da
```

---

## Module Dependency Diagram

```mermaid
graph LR
    subgraph "Header Files"
        GIIS_H[giis.h<br/>Main Header]
        EXT3_H[ext3_fs.h<br/>FS Structures]
        CONFIG_H[config.h<br/>Build Config]
    end

    subgraph "Core Modules"
        MAIN[main.c<br/>Entry Point]
        INIT[init.c<br/>Initialization]
        DIR[dir.c<br/>Directory Ops]
        FILE[file.c<br/>File Ops]
        RECOVERY[get_it_i_say.c<br/>Recovery]
        SEARCH[searchnupdate.c<br/>Search/Update]
        INODE[inode.c<br/>Inode Ops]
        GROUP[group.c<br/>Group Desc]
        UNINIT[uninit.c<br/>Uninstall]
    end

    subgraph "Build System"
        MAKEFILE[Makefile]
        INSTALL[install_giis.sh]
    end

    GIIS_H --> MAIN
    GIIS_H --> INIT
    GIIS_H --> DIR
    GIIS_H --> FILE
    GIIS_H --> RECOVERY
    GIIS_H --> SEARCH
    GIIS_H --> INODE
    GIIS_H --> GROUP
    GIIS_H --> UNINIT

    EXT3_H --> GIIS_H
    CONFIG_H --> GIIS_H

    MAIN --> INIT
    MAIN --> RECOVERY
    MAIN --> SEARCH
    MAIN --> UNINIT

    INIT --> INODE
    INIT --> GROUP
    INIT --> DIR

    RECOVERY --> FILE
    RECOVERY --> INODE

    SEARCH --> DIR
    SEARCH --> FILE

    DIR --> INODE
    DIR --> FILE

    FILE --> INODE

    MAKEFILE --> MAIN
    MAKEFILE --> INIT
    MAKEFILE --> DIR
    MAKEFILE --> FILE
    MAKEFILE --> RECOVERY
    MAKEFILE --> SEARCH
    MAKEFILE --> INODE
    MAKEFILE --> GROUP
    MAKEFILE --> UNINIT

    INSTALL --> MAKEFILE

    style GIIS_H fill:#ffe1e1
    style MAIN fill:#e1f5ff
    style RECOVERY fill:#d4edda
```

---

## Data Structure Diagram

```mermaid
classDiagram
    class file_system {
        +unsigned long group_no_inode_bitmap
        +unsigned long group_no_block_bitmap
        +unsigned long group_no_inode_table
        +unsigned long no_of_inodes_in_group
        +unsigned long no_of_blocks_in_group
        +unsigned long inode_offset
        +unsigned long block_size
        +unsigned long inode_size
        +unsigned long total_inodes
        +unsigned long total_blocks
        +unsigned long free_inodes
        +unsigned long free_blocks
    }

    class s_file_recover_info {
        +char file_name[100]
        +unsigned long inode_no
        +unsigned long parent_inode
        +unsigned long size
        +unsigned long blocks
        +unsigned long mode
        +unsigned long uid
        +unsigned long gid
        +unsigned long atime
        +unsigned long mtime
        +unsigned long ctime
        +unsigned long dtime
        +unsigned long i_block[15]
        +int deleted_flag
    }

    class s_dir_recover_info {
        +char dir_name[100]
        +unsigned long inode_no
        +unsigned long parent_inode
        +unsigned long timestamp
        +int search_flag
    }

    class s_sample_file_data {
        +unsigned long inode_no
        +char sample_data[32]
    }

    class s_rere_info {
        +unsigned long deleted_inode_list[100]
        +char restore_path[100]
        +int count
    }

    class ext3_inode {
        +__u16 i_mode
        +__u16 i_uid
        +__u32 i_size
        +__u32 i_atime
        +__u32 i_ctime
        +__u32 i_mtime
        +__u32 i_dtime
        +__u16 i_gid
        +__u16 i_links_count
        +__u32 i_blocks
        +__u32 i_flags
        +__u32 i_block[15]
    }

    class ext3_super_block {
        +__u32 s_inodes_count
        +__u32 s_blocks_count
        +__u32 s_free_blocks_count
        +__u32 s_free_inodes_count
        +__u32 s_first_data_block
        +__u32 s_log_block_size
        +__u32 s_blocks_per_group
        +__u32 s_inodes_per_group
        +__u16 s_inode_size
    }

    class ext3_group_desc {
        +__u32 bg_block_bitmap
        +__u32 bg_inode_bitmap
        +__u32 bg_inode_table
        +__u16 bg_free_blocks_count
        +__u16 bg_free_inodes_count
        +__u16 bg_used_dirs_count
    }

    file_system --> ext3_super_block : reads from
    file_system --> ext3_group_desc : uses
    s_file_recover_info --> ext3_inode : derived from
    s_file_recover_info --> s_sample_file_data : references
    s_dir_recover_info --> ext3_inode : derived from
```

---

## File System Interaction Diagram

```mermaid
graph TB
    subgraph "GIIS System"
        GIIS[GIIS Tool]
    end

    subgraph "Ext2/Ext3 Filesystem Structure"
        SB[Superblock<br/>Offset: 1024 bytes]
        GDT[Group Descriptor Table]

        subgraph "Block Group 0"
            BB0[Block Bitmap]
            IB0[Inode Bitmap]
            IT0[Inode Table]
            DB0[Data Blocks]
        end

        subgraph "Block Group 1"
            BB1[Block Bitmap]
            IB1[Inode Bitmap]
            IT1[Inode Table]
            DB1[Data Blocks]
        end

        subgraph "Block Group N"
            BBN[Block Bitmap]
            IBN[Inode Bitmap]
            ITN[Inode Table]
            DBN[Data Blocks]
        end
    end

    subgraph "Recovery Process"
        READ[Read Metadata]
        VERIFY[Verify Inode Free]
        COMPARE[Compare Sample]
        RECOVER[Read Data Blocks]
    end

    GIIS -->|1. Read| SB
    SB -->|Block size, Inode count| GIIS

    GIIS -->|2. Read| GDT
    GDT -->|Group info| GIIS

    GIIS -->|3. Access| IT0
    GIIS -->|3. Access| IT1
    GIIS -->|3. Access| ITN

    READ --> IT0

    VERIFY --> IB0
    VERIFY --> IB1
    VERIFY --> IBN

    COMPARE -->|Check first 32 bytes| DB0

    RECOVER -->|Direct blocks| DB0
    RECOVER -->|Indirect blocks| DB0
    RECOVER -->|Double indirect| DB0

    style GIIS fill:#e1f5ff
    style SB fill:#fff4e1
    style GDT fill:#ffe1e1
    style VERIFY fill:#e1ffe1
    style RECOVER fill:#d4edda
```

---

## Deployment Diagram

```mermaid
graph TB
    subgraph "Linux System"
        subgraph "User Space"
            BINARY[/usr/bin/giis<br/>Executable]
            CONFIG[/etc/giis.conf<br/>Configuration]

            subgraph "/usr/share/giis/"
                SYS_DIR[sys/dir<br/>Directory DB]
                SYS_FILE[sys/file<br/>File DB]
                SYS_SIND[sys/sind<br/>Single Indirect DB]
                SYS_DIND[sys/dind<br/>Double Indirect DB]
                SYS_SAMPLE[sys/sample<br/>Sample Data DB]
                GOT_IT[got_it/<br/>Recovered Files]
                UNREC[unrecovered/<br/>Failed Recovery List]
            end

            CRON[/etc/cron.d/<br/>Auto-update Job]
        end

        subgraph "Kernel Space"
            VFS[Virtual File System]
            EXT3[Ext3 Driver]
        end

        subgraph "Hardware"
            DISK[/dev/sdaX<br/>Physical Disk]
        end
    end

    BINARY --> CONFIG
    BINARY --> SYS_DIR
    BINARY --> SYS_FILE
    BINARY --> SYS_SIND
    BINARY --> SYS_DIND
    BINARY --> SYS_SAMPLE
    BINARY --> GOT_IT
    BINARY --> UNREC

    CRON -->|Every 20 min| BINARY

    BINARY -->|System Calls| VFS
    VFS --> EXT3
    EXT3 --> DISK

    style BINARY fill:#e1f5ff
    style CONFIG fill:#fff4e1
    style GOT_IT fill:#d4edda
    style DISK fill:#f0f0f0
```

---

## Recovery Algorithm Flowchart

```mermaid
flowchart TD
    START([Start Recovery]) --> LOAD[Load File Database]
    LOAD --> FILTER[Filter Deleted Files]
    FILTER --> DISPLAY[Display to User]
    DISPLAY --> SELECT{User Selects File}

    SELECT -->|File Selected| GET_INODE[Get Inode Number]
    SELECT -->|Cancel| END([End])

    GET_INODE --> READ_BITMAP[Read Inode Bitmap]
    READ_BITMAP --> CHECK_FREE{Is Inode Free?}

    CHECK_FREE -->|No| REUSED[Inode Reused<br/>Cannot Recover]
    REUSED --> LOG_FAIL[Log to Unrecovered]
    LOG_FAIL --> SELECT

    CHECK_FREE -->|Yes| GET_SAMPLE[Get Sample Data<br/>First 32 Bytes]
    GET_SAMPLE --> READ_DISK[Read Current Disk Data<br/>First 32 Bytes]
    READ_DISK --> COMPARE{Data Matches?}

    COMPARE -->|No| OVERWRITTEN[Data Overwritten]
    OVERWRITTEN --> OFFER_DUMP{Offer GIIS Dump?}
    OFFER_DUMP -->|Yes| FORCE[Force Recovery Mode]
    OFFER_DUMP -->|No| LOG_FAIL

    COMPARE -->|Yes| FORCE
    FORCE --> READ_META[Read Inode Metadata]
    READ_META --> GET_BLOCKS[Get Block Pointers]

    GET_BLOCKS --> DIRECT[Read Direct Blocks<br/>i_block 0-11]
    GET_BLOCKS --> SINGLE[Read Single Indirect<br/>i_block 12]
    GET_BLOCKS --> DOUBLE[Read Double Indirect<br/>i_block 13]

    DIRECT --> WRITE_FILE[Write to got_it/]
    SINGLE --> WRITE_FILE
    DOUBLE --> WRITE_FILE

    WRITE_FILE --> RESTORE_Q{Restore to Original?}

    RESTORE_Q -->|Yes| CHECK_PATH{Original Path Exists?}
    CHECK_PATH -->|Yes| COPY_BACK[Copy to Original Location]
    CHECK_PATH -->|No| CREATE_SYMLINK[Create Symlink]

    RESTORE_Q -->|No| SUCCESS
    COPY_BACK --> SUCCESS[Recovery Successful]
    CREATE_SYMLINK --> SUCCESS

    SUCCESS --> LOG_SUCCESS[Log to restore_details.txt]
    LOG_SUCCESS --> SELECT

    style START fill:#e1f5ff
    style SUCCESS fill:#d4edda
    style REUSED fill:#f8d7da
    style OVERWRITTEN fill:#fff3cd
    style END fill:#e1f5ff
```

---

## State Diagram

```mermaid
stateDiagram-v2
    [*] --> Uninstalled

    Uninstalled --> Installing : giis -i
    Installing --> Initialized : Success
    Installing --> Error : Failure
    Error --> Uninstalled : Fix & Retry

    Initialized --> Monitoring : Cron Active
    Initialized --> Updating : giis -u
    Initialized --> Listing : giis -l
    Initialized --> Recovering : giis -g
    Initialized --> Uninstalling : giis -U

    Monitoring --> Updating : Every 20 min
    Updating --> Initialized : Complete

    Listing --> Initialized : Display Complete

    Recovering --> VerifyingInode : Select File
    VerifyingInode --> CheckingSample : Inode Free
    VerifyingInode --> RecoveryFailed : Inode Reused

    CheckingSample --> ReadingBlocks : Data Matches
    CheckingSample --> ForceDump : Data Overwritten

    ForceDump --> ReadingBlocks : User Confirms
    ForceDump --> RecoveryFailed : User Cancels

    ReadingBlocks --> WritingFile : Blocks Read
    WritingFile --> Restoring : User Wants Restore
    WritingFile --> RecoverySuccess : Keep in got_it/

    Restoring --> RecoverySuccess : Restored

    RecoverySuccess --> Initialized : Continue
    RecoveryFailed --> Initialized : Continue

    Uninstalling --> Uninstalled : Remove Complete
```

---

## Technology Stack

```mermaid
graph TB
    subgraph "Development"
        LANG[C Language<br/>ANSI C + GNU Extensions]
        COMPILER[GCC Compiler<br/>-Wall -O2]
        BUILD[GNU Make]
        INSTALL_SH[Bash Script<br/>install_giis.sh]
    end

    subgraph "System Libraries"
        LIBC[libc<br/>Standard C Library]
        LIBM[libm<br/>Math Library]
        PWD[pwd.h<br/>Password Database]
        UNISTD[unistd.h<br/>POSIX API]
    end

    subgraph "Kernel Interfaces"
        SYSCALL[System Calls<br/>open, read, write, lseek64]
        VFS_INT[VFS Interface]
        EXT3_FS[Ext3 Filesystem Structures]
    end

    subgraph "Platform"
        LINUX[Linux Kernel 2.6+<br/>Updated for 3.x]
        X86_64[x86_64 Architecture]
    end

    LANG --> COMPILER
    COMPILER --> BUILD
    BUILD --> INSTALL_SH

    LANG --> LIBC
    LANG --> LIBM
    LANG --> PWD
    LANG --> UNISTD

    LIBC --> SYSCALL
    UNISTD --> SYSCALL
    SYSCALL --> VFS_INT
    VFS_INT --> EXT3_FS

    EXT3_FS --> LINUX
    LINUX --> X86_64

    style LANG fill:#e1f5ff
    style LINUX fill:#d4edda
```

---

## Configuration and Workflow

```mermaid
flowchart LR
    subgraph "Configuration"
        CONF[giis.conf]
        DEVICE[Device: /dev/sda1]
        DIRS[Directories to Protect]
        TIME[Update Interval: 20 min]
        LEVEL[Max Depth: 10 levels]
    end

    subgraph "Installation"
        COMPILE[make]
        INSTALL[make install]
        SETUP[giis -i]
    end

    subgraph "Monitoring"
        CRON[Cron Job]
        UPDATE[giis -u]
        SNAPSHOT[Update Snapshot]
    end

    subgraph "Recovery"
        LIST[giis -l]
        RECOVER[giis -g]
        DUMP[giis dump]
    end

    CONF --> SETUP
    DEVICE --> CONF
    DIRS --> CONF
    TIME --> CONF
    LEVEL --> CONF

    COMPILE --> INSTALL
    INSTALL --> SETUP
    SETUP --> CRON

    CRON -->|Every TIME minutes| UPDATE
    UPDATE --> SNAPSHOT
    SNAPSHOT --> CRON

    LIST --> RECOVER
    RECOVER --> DUMP

    style SETUP fill:#fff4e1
    style CRON fill:#e1ffe1
    style RECOVER fill:#d4edda
```

---

## Key Design Principles

1. **Snapshot-Based Recovery**: GIIS takes periodic snapshots of filesystem metadata, allowing recovery of files deleted after installation.

2. **Non-Invasive**: The tool reads filesystem structures directly but doesn't modify them during recovery, ensuring safety.

3. **Sample-Based Verification**: Stores first 32 bytes of each file to detect if data has been overwritten.

4. **Modular Architecture**: Clear separation between filesystem operations, data management, and recovery logic.

5. **Efficient Updates**: Cron-based auto-updates only record new/modified files to minimize overhead.

6. **Direct Block Access**: Bypasses the normal file I/O layer to read data from freed inodes.

7. **Flexible Configuration**: Users can specify directories to protect and update frequency.

---

## Limitations

- **Pre-Installation Requirement**: Cannot recover files deleted before GIIS was installed
- **Filesystem Support**: Only works with Ext2/Ext3 (not Ext4, Btrfs, XFS, etc.)
- **Overwrite Detection**: If data blocks are reused, recovery may fail or produce corrupted data
- **Root Access Required**: Needs direct device access to read raw filesystem structures
- **Snapshot Overhead**: Requires periodic scanning and storage of metadata
- **Directory Depth Limit**: Configurable maximum depth (default 10 levels)

---

## Future Enhancement Possibilities

1. Support for Ext4 filesystem
2. GUI interface for easier usage
3. Selective directory monitoring to reduce overhead
4. Compression of snapshot data
5. Network-based recovery for remote systems
6. Integration with filesystem events (inotify) for real-time tracking
7. Support for encrypted filesystems
8. Multi-threaded recovery for faster performance

---

*Generated: 2025-11-07*
*Version: GIIS 4.6.1*
*Author: Lakshmipathi.G*
