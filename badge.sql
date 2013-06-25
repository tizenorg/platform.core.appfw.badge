PRAGMA journal_mode = PERSIST;
        create table if not exists badge_data (
            pkgname TEXT NOT NULL,
            writable_pkgs TEXT,
            badge INTEGER default 0,
            rowid INTEGER PRIMARY KEY AUTOINCREMENT,
            UNIQUE (pkgname)
        );
        create table if not exists badge_option (
            pkgname TEXT NOT NULL,
            display INTEGER default 1,
            UNIQUE (pkgname)
        );
