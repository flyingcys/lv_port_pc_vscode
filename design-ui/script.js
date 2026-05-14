document.addEventListener('DOMContentLoaded', () => {
    // 1. 时钟更新功能
    function updateClock() {
        const now = new Date();
        let hours = now.getHours();
        let minutes = now.getMinutes();
        
        // 格式化为两位数
        hours = hours < 10 ? '0' + hours : hours;
        minutes = minutes < 10 ? '0' + minutes : minutes;
        
        document.getElementById('clock').textContent = `${hours}:${minutes}`;
    }

    // 初始调用并设置每秒更新
    updateClock();
    setInterval(updateClock, 1000);

    // 2. 电池电量模拟 (如果是真实设备可以通过API获取)
    let batteryLevel = 100;
    const batteryText = document.getElementById('battery-text');
    const batteryIcon = document.querySelector('.ph-battery-full');
    
    // 模拟电量消耗
    setInterval(() => {
        if(batteryLevel > 0) {
            batteryLevel -= 1;
            batteryText.textContent = `${batteryLevel}%`;
            
            // 根据电量更新图标
            if(batteryLevel <= 20) {
                batteryIcon.className = 'ph-fill ph-battery-warning';
                batteryIcon.style.color = '#ff4d4f';
            } else if(batteryLevel <= 50) {
                batteryIcon.className = 'ph-fill ph-battery-medium';
            }
        }
    }, 60000); // 每分钟掉1%

    // 3. 滚动与分页指示器联动
    const homeScreen = document.getElementById('home-screen');
    const dots = document.querySelectorAll('.dot');
    
    // 监听滚动事件，更新指示器
    homeScreen.addEventListener('scroll', () => {
        // 计算当前页索引 (四舍五入)
        const scrollLeft = homeScreen.scrollLeft;
        const width = homeScreen.clientWidth;
        const pageIndex = Math.round(scrollLeft / width);
        
        // 更新指示器状态
        dots.forEach((dot, index) => {
            if (index === pageIndex) {
                dot.classList.add('active');
            } else {
                dot.classList.remove('active');
            }
        });
    });

    // 点击指示器跳转到对应页面
    dots.forEach((dot, index) => {
        dot.addEventListener('click', () => {
            const width = homeScreen.clientWidth;
            homeScreen.scrollTo({
                left: index * width,
                behavior: 'smooth'
            });
        });
    });

    // 4. 为应用图标添加点击互动效果
    const appItems = document.querySelectorAll('.app-item');
    appItems.forEach(item => {
        item.addEventListener('click', function() {
            // 触觉/视觉反馈
            this.style.transform = 'scale(0.85)';
            setTimeout(() => {
                this.style.transform = '';
            }, 150);
            
            const appName = this.querySelector('.app-name').textContent;
            console.log(`打开应用: ${appName}`);
            // TODO: 在这里接入实际的App打开逻辑
        });
    });

    // 5. 鼠标拖拽支持 (为了在PC/Web上完美模拟触屏滑动效果)
    let isDown = false;
    let startX;
    let scrollLeft;
    let isDragging = false; // 用于区分是点击还是拖拽

    homeScreen.addEventListener('mousedown', (e) => {
        isDown = true;
        isDragging = false;
        startX = e.pageX - homeScreen.offsetLeft;
        scrollLeft = homeScreen.scrollLeft;
        
        // 拖拽时取消捕捉，让滑动更自由
        homeScreen.style.scrollSnapType = 'none'; 
        homeScreen.style.cursor = 'grabbing';
    });

    homeScreen.addEventListener('mouseleave', () => {
        if (!isDown) return;
        isDown = false;
        resetScrollSnap();
    });

    homeScreen.addEventListener('mouseup', (e) => {
        if (!isDown) return;
        isDown = false;
        resetScrollSnap();
        
        // 阻止拖拽结束时触发点击事件
        if(isDragging) {
            e.preventDefault();
        }
    });

    homeScreen.addEventListener('mousemove', (e) => {
        if (!isDown) return;
        e.preventDefault();
        
        const x = e.pageX - homeScreen.offsetLeft;
        const walk = (x - startX) * 1.5; // 滑动速度倍率
        
        if (Math.abs(walk) > 5) { // 只有移动超过一定像素才认为是拖拽
            isDragging = true;
        }
        
        homeScreen.scrollLeft = scrollLeft - walk;
    });
    
    // 阻止拖拽过程中触发a标签或图标点击
    appItems.forEach(item => {
        item.addEventListener('click', (e) => {
            if(isDragging) {
                e.preventDefault();
                e.stopPropagation();
            }
        });
    });

    function resetScrollSnap() {
        homeScreen.style.cursor = 'default';
        
        const width = homeScreen.clientWidth;
        const currentScroll = homeScreen.scrollLeft;
        const scrollDiff = currentScroll - scrollLeft; // 相对拖拽起始位置的偏移
        const initialPageIndex = Math.round(scrollLeft / width);
        
        let targetIndex = initialPageIndex;
        
        // 优化翻页体验：只要鼠标拖动超过 50 像素，就判定为翻页
        if (scrollDiff > 50) {
            targetIndex = initialPageIndex + 1;
        } else if (scrollDiff < -50) {
            targetIndex = initialPageIndex - 1;
        } else {
            targetIndex = Math.round(currentScroll / width);
        }
        
        // 限制索引范围，防止超出页数
        const maxIndex = document.querySelectorAll('.app-page').length - 1;
        targetIndex = Math.max(0, Math.min(targetIndex, maxIndex));

        homeScreen.scrollTo({
            left: targetIndex * width,
            behavior: 'smooth'
        });

        // 延迟恢复 scroll-snap，防止与平滑滚动产生冲突导致页面又弹回去
        setTimeout(() => {
            if (!isDown) {
                homeScreen.style.scrollSnapType = 'x mandatory';
            }
        }, 400);
    }

    // 6. 上下滑动面板逻辑 (控制中心 & 通知中心)
    const deviceContainer = document.querySelector('.device-container');
    const topPanel = document.getElementById('top-panel');
    const bottomPanel = document.getElementById('bottom-panel');
    
    let startY = 0;
    let startXGlobal = 0;
    let isGlobalDragging = false;
    let panelState = 'closed'; // 'closed', 'top-open', 'bottom-open'

    // 统一处理触屏和鼠标事件
    function getEventPoint(e) {
        return e.touches ? e.touches[0] : e;
    }

    deviceContainer.addEventListener('mousedown', handleGlobalStart);
    deviceContainer.addEventListener('touchstart', handleGlobalStart, {passive: true});

    document.addEventListener('mousemove', handleGlobalMove);
    document.addEventListener('touchmove', handleGlobalMove, {passive: false});

    document.addEventListener('mouseup', handleGlobalEnd);
    document.addEventListener('touchend', handleGlobalEnd);

    function handleGlobalStart(e) {
        const point = getEventPoint(e);
        startY = point.pageY;
        startXGlobal = point.pageX;
        isGlobalDragging = true;
    }

    function handleGlobalMove(e) {
        if (!isGlobalDragging) return;
        
        const point = getEventPoint(e);
        const deltaY = point.pageY - startY;
        const deltaX = point.pageX - startXGlobal;
        
        // 判断是垂直滑动还是水平滑动
        if (Math.abs(deltaY) > Math.abs(deltaX) && Math.abs(deltaY) > 20) {
            // 边缘检测触发
            const rect = deviceContainer.getBoundingClientRect();
            const localStartY = startY - rect.top;
            
            // 下拉控制中心 (从顶部边缘下拉)
            if (deltaY > 50 && panelState === 'closed' && localStartY < 80) {
                openPanel('top');
                isGlobalDragging = false;
            }
            // 上拉收起控制中心
            else if (deltaY < -50 && panelState === 'top-open') {
                closePanels();
                isGlobalDragging = false;
            }
            // 上拉通知中心 (从底部边缘上拉)
            else if (deltaY < -50 && panelState === 'closed' && localStartY > rect.height - 80) {
                openPanel('bottom');
                isGlobalDragging = false;
            }
            // 下拉收起通知中心
            else if (deltaY > 50 && panelState === 'bottom-open') {
                closePanels();
                isGlobalDragging = false;
            }
        }
    }

    function handleGlobalEnd() {
        isGlobalDragging = false;
    }

    function openPanel(type) {
        closePanels();
        if (type === 'top') {
            topPanel.classList.add('open');
            panelState = 'top-open';
        } else if (type === 'bottom') {
            bottomPanel.classList.add('open');
            panelState = 'bottom-open';
        }
    }

    function closePanels() {
        topPanel.classList.remove('open');
        bottomPanel.classList.remove('open');
        panelState = 'closed';
    }

    // 点击把手也能收起面板
    document.querySelectorAll('.panel-handle-container').forEach(handle => {
        handle.addEventListener('click', closePanels);
    });

    // 独立控制中心按钮点击反馈
    document.querySelectorAll('.control-btn').forEach(btn => {
        btn.addEventListener('click', function() {
            this.classList.toggle('active');
        });
    });
});
